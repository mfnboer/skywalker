import QtCore
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import skywalker

ApplicationWindow {
    id: root
    width: 480
    height: 960
    visible: true
    title: "Skywalker"

    onClosing: (event) => {
        if (Qt.platform.os !== "android") {
            return
        }

        // This catches the back-button on Android

        if (currentStack().depth > 1) {
            let item = currentStackItem()

            if (item instanceof SignIn) {
                // The Sign In page should not be popped from the stack.
                // The user must sign in or close the app.

                if (skywalker.sendAppToBackground())
                    event.accepted = false

                return
            }

            event.accepted = false

            if (item instanceof ComposePost) {
                item.cancel()
                return
            }

            popStack()
        }
        else if (stackLayout.currentIndex > 0) {
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
        id: skywalker
        onLoginOk: start()

        onLoginFailed: (error, host, handleOrDid) => {
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

        onResumeSessionFailed: loginActiveUser()

        onSessionExpired: (error) => {
            timelineUpdateTimer.stop() // TODO: why?
            loginActiveUser()
        }

        onStatusMessage: (msg, level) => statusPopup.show(msg, level, level === QEnums.STATUS_LEVEL_INFO ? 2 : 30)
        onPostThreadOk: (modelId, postEntryIndex) => viewPostThread(modelId, postEntryIndex)
        onGetUserProfileOK: () => skywalker.getUserPreferences()

        onGetUserProfileFailed: (error) => {
            console.warn("FAILED TO LOAD USER PROFILE:", error)
            statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
            skywalker.signOut()
            signIn()
        }

        onGetUserPreferencesOK: () => {
            inviteCodeStore.load()
            skywalker.syncTimeline()
        }

        onGetUserPreferencesFailed: {
            console.warn("FAILED TO LOAD USER PREFERENCES")
            statusPopup.show("FAILED TO LOAD USER PREFERENCES", QEnums.STATUS_LEVEL_ERROR)
            skywalker.signOut()
            signIn()
        }

        onTimelineSyncOK: (index) => {
            getTimelineView().setInSync(index)
            timelineUpdateTimer.start()
        }

        onTimelineSyncFailed: {
            console.warn("SYNC FAILED")
            getTimelineView().setInSync(0)
        }

        onTimelineRefreshed: (prevTopPostIndex) => {
            console.debug("Time line refreshed, prev top post index:", prevTopPostIndex)

            if (prevTopPostIndex > 0)
                getTimelineView().moveToPost(prevTopPostIndex - 1)
        }

        onGetDetailedProfileOK: (profile) => {
            let modelId = skywalker.createAuthorFeedModel(profile)
            viewAuthor(profile, modelId)
        }

        function start() {
            skywalker.getUserProfileAndFollows()
        }
    }

    StackLayout {
        id: stackLayout
        anchors.fill: parent
        currentIndex: 0

        StackView {
            id: timelineStack
        }
        StackView {
            id: notificationStack
        }
        StackView {
            id: searchStack
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

    SettingsDrawer {
        id: settingsDrawer
        height: parent.height
        edge: Qt.RightEdge

        onProfile: {
            let did = skywalker.getUserDid()
            skywalker.getDetailedProfile(did)
            close()
        }

        onInviteCodes: {
            let component = Qt.createComponent("InviteCodesView.qml")
            const codes = inviteCodeStore.getCodes()
            let page = component.createObject(root, { codes: codes })
            page.onClosed.connect(() => { popStack() })
            page.onAuthorClicked.connect((did) => { skywalker.getDetailedProfile(did) })
            pushStack(page)
            close()
        }

        onContentFiltering: {
            editContentFilterSettings()
            close()
        }

        onBlockedAccounts: {
            let modelId = skywalker.createAuthorListModel(
                    QEnums.AUTHOR_LIST_BLOCKS, "")
            root.viewAuthorList(modelId, qsTr("Blocked Accounts"),
                    qsTr("Blocked accounts cannot reply in your threads, mention you, or otherwise interact with you. You will not see their content and they will be prevented from seeing yours."),
                    false)
            close()
        }

        onMutedAccounts: {
            let modelId = skywalker.createAuthorListModel(
                    QEnums.AUTHOR_LIST_MUTES, "")
            root.viewAuthorList(modelId, qsTr("Muted Accounts"),
                    qsTr("Muted accounts have their posts removed from your feed and from your notifications. Mutes are completely private."),
                    false)
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
            signOutCurrentUser()
            signIn()
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
                skywalker.login(profile.did, password, host)
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

        Column {
            id: menuColumn
            width: parent.width

            Item {
                width: parent.width
                height: closeButton.height

                SvgButton {
                    id: closeButton
                    anchors.right: parent.right
                    iconColor: guiSettings.textColor
                    Material.background: "transparent"
                    svg: svgOutline.close
                    onClicked: repostDrawer.close()
                }

                Button {
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
                }
            }
            Button {
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

    LinkUtils {
        id: linkUtils
        skywalker: skywalker

        onWebLink: (link) => Qt.openUrlExternally(link)
        onPostLink: (atUri) => skywalker.getPostThread(atUri)
        onAuthorLink: (handle) => skywalker.getDetailedProfile(handle)
    }

    InviteCodeStore {
        id: inviteCodeStore
        skywalker: skywalker
    }

    GuiSettings {
        id: guiSettings
    }

    function showSettingsDrawer() {
        settingsDrawer.show()
    }

    function showSwitchUserDrawer() {
        switchUserDrawer.show()
    }

    function openLink(link) {
        linkUtils.openLink(link)
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
        pushStack(page)
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
                skywalker.login(user, password, host)
        })
        pushStack(page)
    }

    function loginActiveUser() {
        const userSettings = skywalker.getUserSettings()
        const did = userSettings.getActiveUserDid()

        if (did) {
            const host = userSettings.getHost(did)
            const password = userSettings.getPassword(did)

            if (host) {
                skywalker.login(did, password, host)
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
                skywalker.login(handle, password, host)
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
                skywalker.login(profile.did, password, host)
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

    function signOutCurrentUser() {
        timelineUpdateTimer.stop()
        getTimelineView().stopSync()
        unwindStack()
        skywalker.signOut()
    }

    function composePost(initialText) {
        let component = Qt.createComponent("ComposePost.qml")
        let page = component.createObject(root, {
                skywalker: skywalker,
                initialText: initialText
        })
        page.onClosed.connect(() => { popStack() })
        pushStack(page)
    }

    function composeReply(replyToUri, replyToCid, replyToText, replyToDateTime, replyToAuthor,
                          replyRootUri, replyRootCid)
    {
        let component = Qt.createComponent("ComposePost.qml")
        let page = component.createObject(root, {
                skywalker: skywalker,
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
        pushStack(view)
    }

    function viewTimeline() {
        unwindStack()
        stackLayout.currentIndex = 0
    }

    function viewNotifications() {
        unwindStack()
        stackLayout.currentIndex = 1

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

    function viewSearchView() {
        unwindStack()
        stackLayout.currentIndex = 2
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
        skywalker.getAuthorList(modelId, 50)
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
        item.destroy()
    }

    function pushStack(item) {
        currentStack().push(item)
    }

    function unwindStack() {
        while (currentStack().depth > 1)
            popStack()
    }

    Component.onCompleted: {
        let timelineComponent = Qt.createComponent("TimelineView.qml")
        let timelineView = timelineComponent.createObject(root, { skywalker: skywalker })
        timelineStack.push(timelineView)

        let notificationsComponent = Qt.createComponent("NotificationListView.qml")
        let notificationsView = notificationsComponent.createObject(root,
                { skywalker: skywalker, timeline: timelineView })
        notificationsView.onClosed.connect(() => { stackLayout.currentIndex = 0 })
        notificationStack.push(notificationsView)

        let searchComponent = Qt.createComponent("SearchView.qml")
        let searchView = searchComponent.createObject(root,
                { skywalker: skywalker, timeline: timelineView })
        searchView.onClosed.connect(() => { stackLayout.currentIndex = 0 })
        searchStack.push(searchView)

        // Try to resume the previous session. If that fails, then ask the user to login.
        skywalker.resumeSession()
    }
}
