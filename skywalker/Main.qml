import QtCore
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import skywalker

ApplicationWindow {
    property var feedViews: new Map()

    id: root
    width: 480
    height: 960
    visible: true
    title: "Skywalker"

    Accessible.role: Accessible.Application

    onClosing: (event) => {
        if (Qt.platform.os !== "android") {
            return
        }

        // This catches the back-button on Android

        if (currentStack().depth > 1) {
            let item = currentStackItem()

            if (item instanceof SignIn || item instanceof Login || item instanceof StartupStatus) {
                // The Sign In page should not be popped from the stack.
                // The user must sign in or close the app.

                if (skywalker.sendAppToBackground())
                    event.accepted = false

                return
            }

            event.accepted = false

            // TODO: need something better than this
            if (item instanceof ComposePost || item instanceof EditList || item instanceof EditProfile) {
                item.cancel()
                return
            }

            popStack()
        }
        else if (stackLayout.currentIndex === stackLayout.searchIndex ||
                 stackLayout.currentIndex === stackLayout.feedsIndex) {
            // Hack to hide the selection anchor on Android that should not have been
            // there at all.
            currentStackItem().hide()
            event.accepted = false
            viewTimeline()
        }
        else if (stackLayout.currentIndex !== stackLayout.timelineIndex) {
            event.accepted = false
            viewTimeline()
        }
        else if (skywalker.sendAppToBackground()) {
            event.accepted = false
        }
    }

    SvgOutline {
        id: svgOutline
    }
    SvgFilled {
        id: svgFilled
    }

    BusyIndicator {
        id: busyIndicator
        z: 200
        anchors.centerIn: parent
        running: skywalker.getPostThreadInProgress
    }

    StatusPopup {
        id: statusPopup
        y: guiSettings.headerHeight
    }

    Skywalker {
        signal authorFeedOk(int modelId)
        signal authorFeedError(int modelId, string error, string msg)

        id: skywalker

        onLoginOk: start()

        onLoginFailed: (error, host, handleOrDid) => {
            closeStartupStatus()

            if (handleOrDid.startsWith("did:")) {
                const did = handleOrDid
                const userSettings = getUserSettings()
                const user = userSettings.getUser(did)
                loginUser(host, user.handle, did, error)
            } else {
                loginUser(host, handleOrDid, "", error)
            }
        }

        onResumeSessionOk: start()

        onResumeSessionExpired: {
            closeStartupStatus();
            loginActiveUser();
        }

        onResumeSessionFailed: (error) => {
            closeStartupStatus()

            if (error)
                statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)

            signOutCurrentUser()
            signIn()
        }

        onSessionExpired: (error) => {
            closeStartupStatus()
            statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
            signOutCurrentUser()
            signIn()
        }

        onStatusMessage: (msg, level) => statusPopup.show(msg, level, level === QEnums.STATUS_LEVEL_INFO ? 2 : 30)
        onPostThreadOk: (modelId, postEntryIndex) => viewPostThread(modelId, postEntryIndex)
        onGetUserProfileOK: () => skywalker.getUserPreferences()

        onGetUserProfileFailed: (error) => {
            console.warn("FAILED TO LOAD USER PROFILE:", error)
            closeStartupStatus()
            statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
            signOutCurrentUser()
            signIn()
        }

        onGetUserPreferencesOK: () => {
            const did = skywalker.getUserDid()
            let userSettings = skywalker.getUserSettings()
            const lastSignIn = userSettings.getLastSignInTimestamp(did)
            inviteCodeStore.load(lastSignIn)
            skywalker.bookmarks.load(userSettings)
            skywalker.mutedWords.load(userSettings)

            setStartupStatus(qsTr("Rewinding timeline"))
            skywalker.syncTimeline()
            userSettings.updateLastSignInTimestamp(did)
        }

        onGetUserPreferencesFailed: {
            console.warn("FAILED TO LOAD USER PREFERENCES")
            closeStartupStatus()
            statusPopup.show("FAILED TO LOAD USER PREFERENCES", QEnums.STATUS_LEVEL_ERROR)
            signOutCurrentUser()
            signIn()
        }

        onTimelineSyncOK: (index) => {
            closeStartupStatus()
            getTimelineView().setInSync(index)
            timelineUpdateTimer.start()
        }

        onTimelineSyncFailed: {
            console.warn("SYNC FAILED")
            closeStartupStatus()
            getTimelineView().setInSync(0)
        }

        onTimelineRefreshed: (prevTopPostIndex) => {
            console.debug("Time line refreshed, prev top post index:", prevTopPostIndex)

            if (prevTopPostIndex > 0)
                getTimelineView().moveToPost(prevTopPostIndex - 1)
        }

        onGapFilled: (gapEndIndex) => {
            console.debug("Gap filled, end index:", gapEndIndex)
            getTimelineView().moveToPost(gapEndIndex, 0, true)
        }

        onGetDetailedProfileOK: (profile) => {
            let modelId = skywalker.createAuthorFeedModel(profile)
            viewAuthor(profile, modelId)
        }

        onGetAuthorFeedOk: (modelId) => authorFeedOk(modelId)
        onGetAuthorFeedFailed: (modelId, error, msg) => authorFeedError(modelId, error, msg)

        onGetFeedGeneratorOK: (generatorView, viewPosts) => {
            if (viewPosts)
                viewPostFeed(generatorView)
            else
                viewFeedDescription(generatorView)
        }

        onSharedTextReceived: (text) => {
            let item = currentStackItem()

            if (item instanceof ComposePost)
                item.addSharedText(text)
            else if (item instanceof PostThreadView)
                item.reply(text)
            else if (item instanceof AuthorView)
                item.mentionPost(text)
            else
                composePost(text)
        }

        // "file://" or "image://" source
        onSharedImageReceived: (source, text) => {
            let item = currentStackItem()

            if (item instanceof ComposePost)
                item.addSharedPhoto(source, text)
            else if (item instanceof PostThreadView)
                item.reply(text, source)
            else if (item instanceof AuthorView)
                item.mentionPost(text, source)
            else
                composePost(text, source)
        }

        function start() {
            setStartupStatus(qsTr("Loading user profile"))
            skywalker.getUserProfileAndFollows()
        }
    }

    StackLayout {
        readonly property int timelineIndex: 0
        readonly property int notificationIndex: 1
        readonly property int searchIndex: 2
        readonly property int feedsIndex: 3

        id: stackLayout
        anchors.fill: parent
        currentIndex: timelineIndex

        StackView {
            id: timelineStack
        }
        StackView {
            id: notificationStack
        }
        StackView {
            id: searchStack
        }
        StackView {
            id: feedsStack
        }
    }

    Timer {
        id: timelineUpdateTimer
        // There is a trade off: short timeout is fast updating timeline, long timeout
        // allows for better reply thread construction as we receive more posts per update.
        interval: 91000
        running: false
        repeat: true
        onTriggered: {
            skywalker.getTimelinePrepend(2)
            skywalker.updatePostIndexTimestamps()
        }
    }

    // Hack for Talkback
    // When a popup menu (drawer) is shown, then Talkback still lets the user navigate
    // throught the controls on the window underneath. The user may activate a control
    // and the popup stayse open forever!
    // Making this full screen rectangle visible, blocks Talkback from window beneath.
    Rectangle {
        id: popupShield
        anchors.fill: parent
        color: "black"
        opacity: 0.2
        visible: false

        Accessible.role: Accessible.Window
    }

    SettingsDrawer {
        id: settingsDrawer
        height: parent.height
        edge: Qt.RightEdge
        dragMargin: 0
        modal: true

        onAboutToShow: enablePopupShield(true)
        onAboutToHide: enablePopupShield(false)

        onProfile: {
            let did = skywalker.getUserDid()
            skywalker.getDetailedProfile(did)
            close()
        }

        onInviteCodes: {
            let component = Qt.createComponent("InviteCodesView.qml")
            const codes = inviteCodeStore.getCodes()
            const failedToLoad = inviteCodeStore.failedToLoad()
            let page = component.createObject(root, { codes: codes, failedToLoad: failedToLoad })
            page.onClosed.connect(() => { popStack() })
            page.onAuthorClicked.connect((did) => { skywalker.getDetailedProfile(did) })
            pushStack(page)
            close()
        }

        onBookmarks: {
            let component = Qt.createComponent("Bookmarks.qml")
            let page = component.createObject(root, { skywalker: skywalker })
            page.onClosed.connect(() => { popStack() })
            pushStack(page)
            skywalker.getBookmarksPage()
            close()
        }

        onContentFiltering: {
            editContentFilterSettings()
            close()
        }

        onBlockedAccounts: {
            let modelId = skywalker.createAuthorListModel(QEnums.AUTHOR_LIST_BLOCKS, "")
            viewAuthorList(modelId, qsTr("Blocked Accounts"),
                    qsTr("Blocked accounts cannot reply in your threads, mention you, or otherwise interact with you. You will not see their content and they will be prevented from seeing yours."),
                    false)
            close()
        }

        onMutedAccounts: {
            let modelId = skywalker.createAuthorListModel(QEnums.AUTHOR_LIST_MUTES, "")
            viewAuthorList(modelId, qsTr("Muted Accounts"),
                    qsTr("Muted accounts have their posts removed from your feed and from your notifications. Mutes are completely private."),
                    false)
            close()
        }

        onMutedReposts: {
            let userSettings = skywalker.getUserSettings()
            let did = skywalker.getUserDid()
            let listUri = userSettings.getMutedRepostsListUri(did)
            let modelId = skywalker.createAuthorListModel(QEnums.AUTHOR_LIST_LIST_MEMBERS, listUri)
            viewAuthorList(modelId, qsTr("Muted Reposts"),
                    qsTr("Reposts from these accounts are removed from your feed."),
                    false)
            close()
        }

        onModLists: {
            let modelId = skywalker.createListListModel(QEnums.LIST_TYPE_ALL, QEnums.LIST_PURPOSE_MOD, skywalker.getUserDid())
            viewModerationLists(modelId)
            close()
        }

        onUserLists: {
            let modelId = skywalker.createListListModel(QEnums.LIST_TYPE_ALL, QEnums.LIST_PURPOSE_CURATE, skywalker.getUserDid())
            viewUserLists(modelId)
            close()
        }

        onMutedWords: {
            let component = Qt.createComponent("MutedWords.qml")
            let page = component.createObject(root, { skywalker: skywalker })
            page.onClosed.connect(() => { popStack() })
            pushStack(page)
            close()
        }

        onSettings: {
            editSettings()
            close()
        }

        onSwitchAccount: {
            selectUser()
            close()
        }

        onSignOut: {
            skywalker.clearPassword()
            signOutCurrentUser()
            signIn()
            close()
        }

        onAbout: {
            showAbout()
            close()
        }

        function show() {
            user = skywalker.getUser()
            inviteCodeCount = inviteCodeStore.getAvailableCount()
            open()
        }
    }

    SwitchUserDrawer {
        id: switchUserDrawer
        width: parent.width
        height: parent.height * 0.7
        edge: Qt.BottomEdge
        dragMargin: 0
        modal: true

        onAboutToShow: enablePopupShield(true)
        onAboutToHide: enablePopupShield(false)

        onSelectedUser: (profile) => {
            if (!profile.did) {
                signOutCurrentUser()
                newUser()
            }
            else if (profile.did !== skywalker.getUserDid()) {
                signOutCurrentUser()
                const userSettings = skywalker.getUserSettings()
                const host = userSettings.getHost(profile.did)
                const password = userSettings.getPassword(profile.did)

                if (password)
                    skywalkerLogin(profile.did, password, host)
                else
                    loginUser(host, profile.handle, profile.did)
            }

            close()
        }

        function show() {
            const userSettings = skywalker.getUserSettings()
            userList = userSettings.getUserListWithAddAccount()
            open()
        }
    }

    Drawer {
        property string repostedAlreadyUri
        property string repostUri
        property string repostCid
        property string repostText
        property date repostDateTime
        property basicprofile repostAuthor

        id: repostDrawer
        width: parent.width
        edge: Qt.BottomEdge
        dragMargin: 0
        modal: true

        onAboutToShow: enablePopupShield(true)
        onAboutToHide: enablePopupShield(false)

        Column {
            id: menuColumn
            width: parent.width

            Item {
                width: parent.width
                height: closeButton.height

                SvgButton {
                    id: closeButton
                    anchors.right: parent.right
                    svg: svgOutline.close
                    onClicked: repostDrawer.close()

                    Accessible.role: Accessible.Button
                    Accessible.name: qsTr("cancel repost")
                    Accessible.onPressAction: clicked()
                }

                Button {
                    id: repostButton
                    anchors.horizontalCenter: parent.horizontalCenter
                    Material.background: guiSettings.buttonColor
                    contentItem: Text {
                        color: guiSettings.buttonTextColor
                        text: repostDrawer.repostedAlreadyUri ? qsTr("Undo repost") : qsTr("Repost")
                    }
                    onClicked: {
                        if (repostDrawer.repostedAlreadyUri) {
                            postUtils.undoRepost(repostDrawer.repostedAlreadyUri, repostDrawer.repostCid)
                        } else {
                            postUtils.repost(repostDrawer.repostUri, repostDrawer.repostCid)
                        }

                        repostDrawer.close()
                    }

                    Accessible.role: Accessible.Button
                    Accessible.name: contentItem.text
                    Accessible.onPressAction: clicked()

                }
            }
            Button {
                id: quotePostButton
                anchors.horizontalCenter: parent.horizontalCenter
                Material.background: guiSettings.buttonColor
                contentItem: Text {
                    color: guiSettings.buttonTextColor
                    text: qsTr("Quote post")
                }
                onClicked: {
                    root.composeQuote(repostDrawer.repostUri, repostDrawer.repostCid,
                                      repostDrawer.repostText, repostDrawer.repostDateTime,
                                      repostDrawer.repostAuthor)
                    repostDrawer.close()
                }

                Accessible.role: Accessible.Button
                Accessible.name: contentItem.text
                Accessible.onPressAction: clicked()
            }
        }

        function show(hasRepostedUri, uri, cid, text, dateTime, author) {
            repostedAlreadyUri =  hasRepostedUri
            repostUri = uri
            repostCid = cid
            repostText = text
            repostDateTime = dateTime
            repostAuthor = author
            open()
        }
    }

    PostUtils {
        id: postUtils
        skywalker: skywalker

        onRepostOk: statusPopup.show(qsTr("Reposted"), QEnums.STATUS_LEVEL_INFO, 2)
        onRepostFailed: (error) => statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
        onRepostProgress: (msg) => statusPopup.show(qsTr("Reposting"), QEnums.STATUS_LEVEL_INFO)
        onUndoRepostOk: statusPopup.show(qsTr("Repost undone"), QEnums.STATUS_LEVEL_INFO, 2)
        onUndoRepostFailed: (error) => statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
        onLikeFailed: (error) => statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
        onUndoLikeFailed: (error) => statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
    }

    FeedUtils {
        id: feedUtils
        skywalker: skywalker

        onLikeFailed: (error) => statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
        onUndoLikeFailed: (error) => statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
    }

    LinkUtils {
        id: linkUtils
        skywalker: skywalker

        onWebLink: (link) => Qt.openUrlExternally(link)
        onPostLink: (atUri) => skywalker.getPostThread(atUri)
        onFeedLink: (atUri) => skywalker.getFeedGenerator(atUri, true)
        onListLink: (atUri) => viewListByUri(atUri, true)
        onAuthorLink: (handle) => skywalker.getDetailedProfile(handle)
    }

    GraphUtils {
        id: graphUtils
        skywalker: skywalker

        onGetListOk: (list, viewPosts) => {
            if (viewPosts)
                viewList(list)
            else
                viewListFeedDescription(list)
        }
        onGetListFailed: (error) => statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
    }

    InviteCodeStore {
        id: inviteCodeStore
        skywalker: skywalker

        onLoaded: skywalker.notificationListModel.addInviteCodeUsageNofications(inviteCodeStore)
    }

    UnicodeFonts {
        id: unicodeFonts
    }

    GuiSettings {
        id: guiSettings
    }

    function enablePopupShield(enable) {
        popupShield.visible = enable
    }

    function showSettingsDrawer() {
        settingsDrawer.show()
    }

    function showSwitchUserDrawer() {
        switchUserDrawer.show()
    }

    function openLink(link) {
        if (link.startsWith("@")) {
            console.debug("@-MENTION:", link)
            skywalker.getDetailedProfile(link.slice(1))
        } else if (link.startsWith("did:")) {
            console.debug("DID-MENTION:", link)
            skywalker.getDetailedProfile(link)
        } else if (link.startsWith("#")) {
            console.debug("#-TAG:", link)
            viewSearchView(link)
        } else {
            linkUtils.openLink(link)
        }
    }

    function showStartupStatus() {
        let component = Qt.createComponent("StartupStatus.qml")
        let page = component.createObject(root)
        page.setStatus(qsTr("Connecting"))
        pushStack(page, StackView.Immediate)
    }

    function setStartupStatus(msg) {
        let item = currentStackItem()

        if (item instanceof StartupStatus) {
            item.setStatus(msg)
        }
    }

    function closeStartupStatus() {
        let item = currentStackItem()

        if (item instanceof StartupStatus) {
            popStack()
        }
    }

    function showAbout() {
        let component = Qt.createComponent("About.qml")
        let page = component.createObject(root)
        page.onClosed.connect(() => { popStack() })
        pushStack(page)
    }

    function signIn() {
        let component = Qt.createComponent("SignIn.qml")
        let page = component.createObject(root)
        page.onSignIn.connect(() => {
            popStack()

            const userSettings = skywalker.getUserSettings()
            const userList = userSettings.getUserList()

            if (userList.length > 0) {
                selectUser()
                return
            }

            newUser()
        })
        pushStack(page, StackView.Immediate)
    }

    function loginUser(host, handle, did, error="") {
        let component = Qt.createComponent("Login.qml")
        let page = component.createObject(root, { host: host, user: handle, did:did, error: error })
        page.onCanceled.connect(() => {
                popStack()
                signIn()
        })
        page.onAccepted.connect((host, handle, password, did) => {
                popStack()
                const user = did ? did : handle
                skywalkerLogin(user, password, host)
        })
        pushStack(page)
    }

    function loginActiveUser() {
        const userSettings = skywalker.getUserSettings()
        const did = userSettings.getActiveUserDid()

        if (did) {
            const host = userSettings.getHost(did)
            const password = userSettings.getPassword(did)

            if (host && password) {
                skywalkerLogin(did, password, host)
                return
            }
        }

        signIn()
    }

    function newUser() {
        let component = Qt.createComponent("Login.qml")
        let page = component.createObject(root)
        page.onCanceled.connect(() => {
                popStack()
                signIn()
        })
        page.onAccepted.connect((host, handle, password, did) => {
                popStack()
                skywalkerLogin(handle, password, host)
        })

        pushStack(page)
    }

    function selectUser() {
        const userSettings = skywalker.getUserSettings()
        const userList = userSettings.getUserListWithAddAccount()

        let component = Qt.createComponent("SelectSignInUser.qml")
        let page = component.createObject(root, { userList: userList })
        page.onCanceled.connect(() => {
                popStack()

                if (!skywalker.isSignedIn())
                    signIn()
        })
        page.onSelectedUser.connect((profile) => {
                popStack()

                if (skywalker.isSignedIn() && profile.did === skywalker.getUserDid())
                    return
                else
                    signOutCurrentUser()

                if (!profile.did) {
                    newUser()
                    return
                }

                const userSettings = skywalker.getUserSettings()
                const host = userSettings.getHost(profile.did)
                const password = userSettings.getPassword(profile.did)

                if (!password) {
                    loginUser(host, profile.handleOrDid, profile.did)
                    return
                }

                skywalkerLogin(profile.did, password, host)
        })
        page.onDeletedUser.connect((profile) => {
                popStack()

                if (profile.did) {
                    if (profile.did === skywalker.getUserDid())
                        signOutCurrentUser()

                    let userSettings = skywalker.getUserSettings()
                    userSettings.removeUser(profile.did)
                }

                if (!skywalker.isSignedIn())
                    selectUser()
        })
        pushStack(page)
    }

    function skywalkerLogin(user, password, host) {
        showStartupStatus()
        skywalker.login(user, password, host)
    }

    function signOutCurrentUser() {
        timelineUpdateTimer.stop()
        getTimelineView().stopSync()
        unwindStack()
        destroySearchView()
        destroyFeedsView()
        inviteCodeStore.clear()
        skywalker.signOut()
    }

    function composePost(initialText = "", imageSource = "") {
        let component = Qt.createComponent("ComposePost.qml")
        let page = component.createObject(root, {
                skywalker: skywalker,
                initialText: initialText,
                initialImage: imageSource
        })
        page.onClosed.connect(() => { popStack() })
        pushStack(page)
    }

    function composeReply(replyToUri, replyToCid, replyToText, replyToDateTime, replyToAuthor,
                          replyRootUri, replyRootCid, initialText = "", imageSource = "")
    {
        let component = Qt.createComponent("ComposePost.qml")
        let page = component.createObject(root, {
                skywalker: skywalker,
                initialText: initialText,
                initialImage: imageSource,
                replyToPostUri: replyToUri,
                replyToPostCid: replyToCid,
                replyRootPostUri: replyRootUri,
                replyRootPostCid: replyRootCid,
                replyToPostText: replyToText,
                replyToPostDateTime: replyToDateTime,
                replyToAuthor: replyToAuthor
        })
        page.onClosed.connect(() => { popStack() })
        pushStack(page)
    }

    function composeQuote(quoteUri, quoteCid, quoteText, quoteDateTime, quoteAuthor) {
        let component = Qt.createComponent("ComposePost.qml")
        let page = component.createObject(root, {
                skywalker: skywalker,
                openedAsQuotePost: true,
                quoteUri: quoteUri,
                quoteCid: quoteCid,
                quoteText: quoteText,
                quoteDateTime: quoteDateTime,
                quoteAuthor: quoteAuthor
        })
        page.onClosed.connect(() => { popStack() })
        pushStack(page)
    }

    function repost(repostUri, uri, cid, text, dateTime, author) {
        repostDrawer.show(repostUri, uri, cid, text, dateTime, author)
    }

    function like(likeUri, uri, cid) {
        if (likeUri)
            postUtils.undoLike(likeUri, cid)
        else
            postUtils.like(uri, cid)
    }

    function likeFeed(likeUri, uri, cid) {
        if (likeUri)
            feedUtils.undoLike(likeUri, cid)
        else
            feedUtils.like(uri, cid)
    }

    function deletePost(uri, cid) {
        postUtils.deletePost(uri, cid)
    }

    function viewPostThread(modelId, postEntryIndex) {
        let component = Qt.createComponent("PostThreadView.qml")
        let view = component.createObject(root, { modelId: modelId, postEntryIndex: postEntryIndex })
        view.onClosed.connect(() => { popStack() })
        pushStack(view)
    }

    function viewFullImage(imageList, currentIndex) {
        let component = Qt.createComponent("FullImageView.qml")
        let view = component.createObject(root, { images: imageList, imageIndex: currentIndex })
        view.onClosed.connect(() => { popStack() })
        view.onSaveImage.connect((sourceUrl) => { postUtils.savePhoto(sourceUrl) })
        pushStack(view)
    }

    function viewFullAnimatedImage(imageUrl, imageTitle) {
        let component = Qt.createComponent("FullAnimatedImageView.qml")
        let view = component.createObject(root, { imageUrl: imageUrl, imageTitle: imageTitle })
        view.onClosed.connect(() => { popStack() })
        pushStack(view)
    }

    function viewTimeline() {
        unwindStack()
        stackLayout.currentIndex = stackLayout.timelineIndex
    }

    function viewNotifications() {
        unwindStack()
        stackLayout.currentIndex = stackLayout.notificationIndex

        let loadCount = 25
        if (skywalker.unreadNotificationCount > 0) {
            if (skywalker.unreadNotificationCount > loadCount)
                loadCount = Math.min(skywalker.unreadNotificationCount + 5, 100)

            skywalker.getNotifications(loadCount, true)
        }
        else if (!skywalker.notificationListModel.notificationsLoaded()) {
            skywalker.getNotifications(loadCount)
        }
    }

    function createSearchView() {
        let searchComponent = Qt.createComponent("SearchView.qml")
        let searchView = searchComponent.createObject(root,
                { skywalker: skywalker, timeline: getTimelineView(), })
        searchView.onClosed.connect(() => { stackLayout.currentIndex = stackLayout.timelineIndex })
        searchStack.push(searchView)
    }

    function destroySearchView() {
        if (searchStack.depth > 0) {
            let item = searchStack.get(0)
            item.forceDestroy()
            searchStack.clear()
        }
    }

    function viewSearchView(searchText = "") {
        unwindStack()
        stackLayout.currentIndex = stackLayout.searchIndex

        if (searchStack.depth === 0)
            createSearchView()

        currentStackItem().show(searchText)
    }

    function createFeedsView() {
        let feedsComponent = Qt.createComponent("SearchFeeds.qml")
        let feedsView = feedsComponent.createObject(root,
                { skywalker: skywalker, timeline: getTimelineView() })
        feedsView.onClosed.connect(() => { stackLayout.currentIndex = stackLayout.timelineIndex })
        feedsStack.push(feedsView)
    }

    function destroyFeedsView() {
        if (feedsStack.depth > 0) {
            let item = feedsStack.get(0)
            item.forceDestroy()
            feedsStack.clear()
        }

        for (let view of feedViews.values()) {
            view.forceDestroy()
        }

        feedViews.clear()
    }

    function viewFeedsView() {
        unwindStack()
        stackLayout.currentIndex = stackLayout.feedsIndex

        if (feedsStack.depth === 0)
            createFeedsView()

        currentStackItem().show()
    }

    function viewFeed(generatorView) {
        let view = null

        if (root.feedViews.has(generatorView.uri)) {
            view = feedViews.get(generatorView.uri)
            const visibleItem = currentStackItem()

            if (visibleItem === view)
            {
                console.debug("Feed already showing:", generatorView.displayName)
                return
            }

            if (view.atYBeginning) {
                console.debug("Reload feed:", generatorView.displayName)
                skywalker.getFeed(view.modelId)
            }
        }
        else {
            const modelId = skywalker.createPostFeedModel(generatorView)
            skywalker.getFeed(modelId)
            let component = Qt.createComponent("PostFeedView.qml")
            view = component.createObject(root, { skywalker: skywalker, modelId: modelId, showAsHome: true })
            feedViews.set(generatorView.uri, view)
        }

        viewTimeline()
        pushStack(view)
    }

    function viewListFeed(listView) {
        let view = null

        if (root.feedViews.has(listView.uri)) {
            view = feedViews.get(listView.uri)
            const visibleItem = currentStackItem()

            if (visibleItem === view)
            {
                console.debug("List feed already showing:", listView.name)
                return
            }

            if (view.atYBeginning) {
                console.debug("Reload list feed:", listView.name)
                skywalker.getListFeed(view.modelId)
            }
        }
        else {
            const modelId = skywalker.createPostFeedModel(listView)
            skywalker.getListFeed(modelId)
            let component = Qt.createComponent("PostListFeedView.qml")
            view = component.createObject(root, { skywalker: skywalker, modelId: modelId, showAsHome: true })
            feedViews.set(listView.uri, view)
        }

        viewTimeline()
        pushStack(view)
    }

    function viewPostFeed(feed) {
        const modelId = skywalker.createPostFeedModel(feed)
        skywalker.getFeed(modelId)
        let component = Qt.createComponent("PostFeedView.qml")
        let view = component.createObject(root, { skywalker: skywalker, modelId: modelId })
        view.onClosed.connect(() => { popStack() })
        root.pushStack(view)
    }

    function viewListByUri(listUri, viewPosts) {
        graphUtils.getListView(listUri, viewPosts)
    }

    function viewList(list) {
        switch (list.purpose) {
        case QEnums.LIST_PURPOSE_CURATE:
            viewPostListFeed(list)
            break
        case QEnums.LIST_PURPOSE_MOD:
            viewListFeedDescription(list)
            break
        }
    }

    function viewPostListFeed(list) {
        const modelId = skywalker.createPostFeedModel(list)
        skywalker.getListFeed(modelId)
        let component = Qt.createComponent("PostListFeedView.qml")
        let view = component.createObject(root, { skywalker: skywalker, modelId: modelId })
        view.onClosed.connect(() => { popStack() })
        root.pushStack(view)
    }

    function viewAuthor(profile, modelId) {
        let component = Qt.createComponent("AuthorView.qml")
        let view = component.createObject(root, { author: profile, modelId: modelId, skywalker: skywalker })
        view.onClosed.connect(() => { popStack() })
        pushStack(view)
    }

    function viewAuthorList(modelId, title, description = "", showFollow = true) {
        let component = Qt.createComponent("AuthorListView.qml")
        let view = component.createObject(root, {
                title: title,
                modelId: modelId,
                skywalker: skywalker,
                description: description,
                showFollow: showFollow
        })
        view.onClosed.connect(() => { popStack() })
        pushStack(view)
        skywalker.getAuthorList(modelId)
    }

    function viewUserLists(modelId) {
        let component = Qt.createComponent("UserListsPage.qml")
        let page = component.createObject(root, {
                modelId: modelId,
                skywalker: skywalker
        })
        page.onClosed.connect(() => { popStack() })
        pushStack(page)
        skywalker.getListList(modelId)
    }

    function viewModerationLists(modelId) {
        let component = Qt.createComponent("ModerationListsPage.qml")
        let page = component.createObject(root, {
                modelId: modelId,
                skywalker: skywalker
        })
        page.onClosed.connect(() => { popStack() })
        pushStack(page)
        skywalker.getListList(modelId)
    }

    function viewFeedDescription(feed) {
        let component = Qt.createComponent("FeedDescriptionView.qml")
        let view = component.createObject(root, { feed: feed, skywalker: skywalker })
        view.onClosed.connect(() => { popStack() })
        pushStack(view)
    }

    function viewListFeedDescription(list) {
        let component = Qt.createComponent("ListFeedDescriptionView.qml")
        let view = component.createObject(root, { list: list, skywalker: skywalker })
        view.onClosed.connect(() => { popStack() })
        pushStack(view)
    }

    function editSettings() {
        let component = Qt.createComponent("SettingsForm.qml")
        let userPrefs = skywalker.getEditUserPreferences()
        let form = component.createObject(root, { userPrefs: userPrefs })
        form.onClosed.connect(() => { popStack() })
        pushStack(form)
    }

    function editContentFilterSettings() {
        let component = Qt.createComponent("ContentFilterSettings.qml")
        let contentGroupListModel = skywalker.getContentGroupListModel()
        let form = component.createObject(root, { model: contentGroupListModel })
        form.onClosed.connect(() => { popStack() })
        pushStack(form)
    }

    function reportAuthor(author) {
        let component = Qt.createComponent("Report.qml")
        let form = component.createObject(root, { skywalker: skywalker, author: author })
        form.onClosed.connect(() => { popStack() })
        pushStack(form)
    }

    function reportPost(postUri, postCid, postText, postDateTime, author) {
        let component = Qt.createComponent("Report.qml")
        let form = component.createObject(root, {
                skywalker: skywalker,
                postUri: postUri,
                postCid: postCid,
                postText: postText,
                postDateTime: postDateTime,
                author: author })
        form.onClosed.connect(() => { popStack() })
        pushStack(form)
    }

    function reportFeed(feed) {
        let component = Qt.createComponent("Report.qml")
        let form = component.createObject(root, { skywalker: skywalker, feed: feed })
        form.onClosed.connect(() => { popStack() })
        pushStack(form)
    }

    function reportList(list) {
        let component = Qt.createComponent("Report.qml")
        let form = component.createObject(root, { skywalker: skywalker, list: list })
        form.onClosed.connect(() => { popStack() })
        pushStack(form)
    }

    function translateText(text) {
        const lang = Qt.locale().name.split("_")[0]
        const normalized = unicodeFonts.normalizeToNFKD(text)
        const encoded = encodeURIComponent(normalized)
        const url = `https://translate.google.com/?hl=${lang}&sl=auto&tl=${lang}&text=${encoded}&op=translate`
        Qt.openUrlExternally(url)
    }

    function getTimelineView() {
        return timelineStack.get(0)
    }

    function getNotificationsView() {
        return notificationStack.get(0)
    }

    function currentStack() {
        return stackLayout.children[stackLayout.currentIndex]
    }

    function currentStackItem() {
        let stack = currentStack()

        if (stack.depth > 0)
            return stack.get(stack.depth - 1)

        return null
    }

    function popStack() {
        let item = currentStack().pop()

        // PostFeedViews and PostListFeedViews, shown as home, are kept alive in root.feedViews
        if (!((item instanceof PostFeedView || item instanceof PostListFeedView) && item.showAsHome))
            item.destroy()
    }

    function pushStack(item, operation) {
        currentStack().push(item, operation)
    }

    function unwindStack() {
        while (currentStack().depth > 1)
            popStack()
    }

    function setDisplayMode(displayMode) {
        switch (displayMode) {
        case QEnums.DISPLAY_MODE_SYSTEM:
            root.Material.theme = Material.System
            break
        case QEnums.DISPLAY_MODE_LIGHT:
            root.Material.theme = Material.Light
            break
        case QEnums.DISPLAY_MODE_DARK:
            root.Material.theme = Material.Dark
            break
        default:
            root.Material.theme = Material.System
            break
        }

        console.debug("Theme set to:", root.Material.theme)
        root.Material.accent = guiSettings.accentColor
        let userSettings = skywalker.getUserSettings()
        userSettings.setLinkColor(guiSettings.linkColor)
    }

    function getSkywalker() {
        return skywalker
    }

    Component.onCompleted: {
        console.debug("DPR:", Screen.devicePixelRatio)
        console.debug("Font pt:", Qt.application.font.pointSize)
        console.debug("Font px:", Qt.application.font.pixelSize)
        console.debug(Qt.fontFamilies());

        const userSettings = skywalker.getUserSettings()
        setDisplayMode(userSettings.getDisplayMode())

        let timelineComponent = Qt.createComponent("TimelineView.qml")
        let timelineView = timelineComponent.createObject(root, { skywalker: skywalker })
        timelineStack.push(timelineView)

        let notificationsComponent = Qt.createComponent("NotificationListView.qml")
        let notificationsView = notificationsComponent.createObject(root,
                { skywalker: skywalker, timeline: timelineView })
        notificationsView.onClosed.connect(() => { stackLayout.currentIndex = stackLayout.timelineIndex })
        notificationStack.push(notificationsView)

        showStartupStatus()

        // Try to resume the previous session. If that fails, then ask the user to login.
        skywalker.resumeSession()
    }
}
