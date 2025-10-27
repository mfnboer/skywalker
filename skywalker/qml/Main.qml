import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import skywalker

ApplicationWindow {
    property double postButtonRelativeX: 1.0
    readonly property bool isPortrait: width < height
    readonly property bool showSideBar: !isPortrait && skywalker.getUserSettings().landscapeSideBar && sideBar.width >= guiSettings.sideBarMinWidth
    property var didSkywalkerMap: new Map()
    property var didPostUtilsMap: new Map()
    property var didProfileUtilsMap: new Map()
    property var didGraphUtilsMap: new Map()
    property var didFeedUtilsMap: new Map()
    property var didLinkUtilsMap: new Map()

    // Monitor FPS. Qt6.9 brings QFrameTimer
    // property double frameIntervalStart: Date.now()
    // property double prevFrame: frameIntervalStart
    // property double minFrameMs: 1000
    // property double maxFrameMs: 0
    // property int frameCount: 0

    id: root
    width: 480
    height: 960
    visible: true
    title: skywalker.APP_NAME // qmllint disable missing-property
    color: guiSettings.backgroundColor

    // Added for Qt6.9 to make all the changes for full screen display in Android 15
    // work. Instead of those changes, using the SafeArea option may be nicer.
    topPadding: 0
    bottomPadding: 0
    leftPadding: 0
    rightPadding: 0

    // Monitor FPS. Qt6.9 brings QFrameTimer
    // onFrameSwapped: {
    //     const nextFrame = Date.now()
    //     const frameDt = nextFrame - prevFrame
    //     const dt = nextFrame - frameIntervalStart

    //     minFrameMs = Math.min(frameDt, minFrameMs)
    //     maxFrameMs = Math.max(frameDt, maxFrameMs)
    //     prevFrame = nextFrame
    //     ++frameCount

    //     if (dt > 1000) {
    //         const fps = Math.round(frameCount / dt * 1000)
    //         console.debug("FPS:", fps, "DT:", dt, "MIN:", minFrameMs, "MAX:", maxFrameMs)
    //         frameCount = 0
    //         frameIntervalStart = nextFrame
    //         minFrameMs = 1000
    //         maxFrameMs = 0
    //     }
    // }

    onPostButtonRelativeXChanged: {
        let settings = root.getSkywalker().getUserSettings()
        settings.setPostButtonRelativeX(postButtonRelativeX)
    }

    onIsPortraitChanged: {
        guiSettings.updateScreenMargins()

        // HACK: the light/dark mode of the status bar gets lost when orientation changes
        displayUtils.resetStatusBarLightMode()
    }

    onClosing: (event) => {
        if (Qt.platform.os !== "android") {
            return
        }

        // This catches the back-button on Android

        if (utils.isEmojiPickerShown()) {
            utils.dismissEmojiPicker()
            event.accepted = false
        }
        else if (currentStack().depth > 1) {
            let item = currentStackItem()

            if (item instanceof SignIn || item instanceof Login || item instanceof StartupStatus) {
                // The Sign In page should not be popped from the stack.
                // The user must sign in or close the app.

                if (displayUtils.sendAppToBackground())
                    event.accepted = false

                return
            }

            event.accepted = false

            if (typeof item.cancel == 'function') {
                item.cancel()
                return
            }

            if (typeof item.closeTransition == 'number')
                popStack(null, item.closeTransition)
            else
                popStack()
        }
        else if (rootContent.currentIndex === rootContent.searchIndex) {
            // Hack to hide the selection anchor on Android that should not have been
            // there at all.
            currentStackItem().hide()
            event.accepted = false
            viewTimeline()
        }
        else if (rootContent.currentIndex !== rootContent.timelineIndex) {
            event.accepted = false
            viewTimeline()
        }
        else if (favoritesTabBar.currentIndex !== 0) {
            event.accepted = false
            favoritesTabBar.setCurrentIndex(0)
        }
        else if (displayUtils.sendAppToBackground()) {
            event.accepted = false
        }
    }

    FontLoader {
        source: UnicodeFonts.getEmojiFontSource() // qmllint disable missing-property
        onStatusChanged: console.debug("Font loader status:", status, name, source)
    }

    BusyIndicator {
        id: busyIndicator
        z: 200
        anchors.centerIn: parent
        running: skywalker.getPostThreadInProgress || skywalker.getDetailedProfileInProgress
    }

    StatusPopup {
        id: statusPopup
        x: rootContent.x
        y: guiSettings.headerHeight
        width: rootContent.width
    }

    FavoritesTabBar {
        property var favoritesSwipeView
        property bool favoritesSwipeViewVisible: false
        property bool show: favoritesSwipeViewVisible && skywalker.getUserSettings().favoritesBarPosition !== QEnums.FAVORITES_BAR_POSITION_NONE

        id: favoritesTabBar
        x: sideBar.visible ? sideBar.x + sideBar.width : 0
        y: (favoritesSwipeView && favoritesSwipeView.currentView) ? favoritesSwipeView.currentView.favoritesY : 0
        z: guiSettings.headerZLevel - 1
        width: parent.width - x - guiSettings.rightMargin
        position: skywalker.getUserSettings().favoritesBarPosition === QEnums.FAVORITES_BAR_POSITION_BOTTOM ? TabBar.Footer : TabBar.Header
        favoriteFeeds: skywalker.favoriteFeeds
        clip: true
        visible: show && favoriteFeeds.userOrderedPinnedFeeds.length > 0

        onCurrentIndexChanged: {
            if (currentIndex < 0)
                return

            const item = contentChildren[currentIndex]

            if (item && item instanceof SkySettingsTabButton) {
                if (favoritesSwipeView)
                    Qt.callLater(() => { setCurrentIndex(favoritesSwipeView.currentIndex) })
                else
                    Qt.callLater(() => { setCurrentIndex(0) })
            }
            else if (favoritesSwipeView) {
                favoritesSwipeView.setCurrentIndex(currentIndex)
            }
        }

        function update() {
            let view = currentStackItem()
            favoritesSwipeViewVisible = (view instanceof FavoritesSwipeView)
        }
    }

    footer: SkyFooter {
        property var favoritesSwipeView: favoritesTabBar.favoritesSwipeView

        width: parent.width - guiSettings.rightMargin
        timeline: favoritesSwipeView ? favoritesSwipeView.currentView : null
        skywalker: root.getSkywalker()
        activePage: QEnums.UI_PAGE_HOME
        extraFooterMargin: getExtraFooterMargin()
        onHomeClicked: favoritesSwipeView.currentView.moveToHome()
        onNotificationsClicked: viewNotifications()
        onSearchClicked: viewSearchView()
        onMessagesClicked: viewChat()
        footerVisible: !showSideBar
        visible: favoritesTabBar.favoritesSwipeViewVisible

        function getExtraFooterMargin() {
            if (favoritesTabBar.position == TabBar.Footer)
                return favoritesTabBar.visible ? y - favoritesTabBar.y : 0
            else
                return favoritesSwipeView && favoritesSwipeView.currentView ? favoritesSwipeView.currentView.extraFooterMargin : 0
        }
    }

    function isFavoritesTabBarVisible() {
        return favoritesTabBar.visible
    }

    function showFavoritesSorter() {
        let component = guiSettings.createComponent("FavoritesSorter.qml")
        let page = component.createObject(root, { favoriteFeeds: skywalker.favoriteFeeds })
        page.onClosed.connect(() => { root.popStack() })
        root.pushStack(page)
    }

    function showEmojiNamesList(txt) {
        let component = guiSettings.createComponent("EmojiNamesList.qml")
        let page = component.createObject(rootContent, { txt: txt })
        page.onAccepted.connect(() => { page.destroy() })
        page.onRejected.connect(() => { page.destroy() })
        page.open()
    }

    function clearStatusMessage() {
        statusPopup.close()
    }

    function syncTimelineToPost(postIndex, offsetY = 0) {
        skywalker.timelineModel.addFilteredPostFeedModelsFromSettings()
        getTimelineView().switchToFullView()
        closeStartupStatus()
        getTimelineView().setInSync(postIndex, offsetY)
        skywalker.startTimelineAutoUpdate()
    }

    function showLastViewedFeed() {
        let userSettings = skywalker.getUserSettings()
        const lastViewed = userSettings.getLastViewedFeed(skywalker.getUserDid())

        console.debug("Show last viewed feed:", lastViewed)
        getFavoritesSwipeView().trackLastViewedFeed = true

        if (lastViewed === "home")
            return

        let favorite = null

        if (lastViewed.startsWith("at://"))
            favorite = skywalker.favoriteFeeds.getPinnedFeed(lastViewed)
        else
            favorite = skywalker.favoriteFeeds.getPinnedSearch(lastViewed)

        if (favorite.isNull())
        {
            console.debug("Last viewed feed not in favorites:", lastViewed)
            return
        }

        showFavorite(favorite)
    }

    function showFavorite(favorite) {
        console.debug("Show favorite:", favorite)
        favoritesTabBar.setCurrent(favorite)
    }

    Skywalker {
        id: skywalker

        onSkywalkerCreated: (did, sw) => {
            console.debug("Skywalker created:", did)
            didSkywalkerMap.set(did, sw)

            const puComp = guiSettings.createComponent("MainPostUtils.qml")
            const pu = puComp.createObject(root, { skywalker: sw })
            didPostUtilsMap.set(did, pu)

            const profComp = guiSettings.createComponent("MainProfileUtils.qml")
            const profUtils = profComp.createObject(root, { skywalker: sw })
            didProfileUtilsMap.set(did, profUtils)

            const guComp = guiSettings.createComponent("MainGraphUtils.qml")
            const gu = guComp.createObject(root, { skywalker: sw })
            didGraphUtilsMap.set(did, gu)

            const fuComp = guiSettings.createComponent("MainFeedUtils.qml")
            const fu = fuComp.createObject(root, { skywalker: sw })
            didFeedUtilsMap.set(did, fu)

            const luComp = guiSettings.createComponent("MainLinkUtils.qml")
            const lu = luComp.createObject(root, { skywalker: sw })
            didLinkUtilsMap.set(did, lu)
        }

        onSkywalkerDestroyed: (did) => {
            console.debug("Skywalker destroyed:", did)
            didSkywalkerMap.delete(did)
            didPostUtilsMap.delete(did)
            didProfileUtilsMap.delete(did)
            didGraphUtilsMap.delete(did)
            didFeedUtilsMap.delete(did)
            didLinkUtilsMap.delete(did)
        }

        onLoginOk: start()

        onLoginFailed: (error, msg, host, handleOrDid, password) => {
            closeStartupStatus()

            if (handleOrDid.startsWith("did:")) {
                const did = handleOrDid
                const userSettings = getUserSettings()
                const user = userSettings.getUser(did)
                loginUser(host, user.handle, did, error, msg, password)
            } else {
                loginUser(host, handleOrDid, "", error, msg, password)
            }
        }

        onResumeSessionOk: start()

        onResumeSessionFailed: (error) => {
            closeStartupStatus()

            if (skywalker.autoLogin()) {
                showStartupStatus()
                return
            }

            if (error)
                statusPopup.show(getUserDid(), error, QEnums.STATUS_LEVEL_ERROR)

            signOutCurrentUser()
            signIn()
        }

        onSessionExpired: (error) => {
            closeStartupStatus()
            statusPopup.show(getUserDid(), error, QEnums.STATUS_LEVEL_ERROR)
            signOutCurrentUser()
            signIn()
        }

        onSessionDeleted: {
            signOutCurrentUser()
            signIn()
        }

        onStatusMessage: (did, msg, level, seconds) => { // qmllint disable signal-handler-parameters
                const period = seconds > 0 ? seconds : (level === QEnums.STATUS_LEVEL_INFO ? 2 : 30)
                statusPopup.show(did, msg, level, period)
        }

        onStatusClear: statusPopup.clear()

        onPostThreadOk: (did, modelId, postEntryIndex) => viewPostThread(did, modelId, postEntryIndex)
        onGetUserProfileOK: () => skywalker.getUserPreferences()

        onGetUserProfileFailed: (error) => {
            console.warn("FAILED TO LOAD USER PROFILE:", error)
            closeStartupStatus()
            statusPopup.show(getUserDid(), error, QEnums.STATUS_LEVEL_ERROR)
            signOutCurrentUser()
            signIn()
        }

        onGetUserPreferencesOK: () => skywalker.dataMigration();

        onDataMigrationStatus: (status) => setStartupStatus(status)

        onDataMigrationDone: () => {
            const did = skywalker.getUserDid()
            let userSettings = skywalker.getUserSettings()
            const lastSignIn = userSettings.getLastSignInTimestamp(did)
            // inviteCodeStore.load(lastSignIn)
            skywalker.loadMutedWords()
            skywalker.loadHashtags()
            skywalker.focusHashtags.load(skywalker.getUserDid(), skywalker.getUserSettings())
            skywalker.chat.getAllConvos()
            setStartupStatus(qsTr("Rewinding timeline"))
            skywalker.syncTimeline()
            userSettings.updateLastSignInTimestamp(did)
        }

        onGetUserPreferencesFailed: (error) => {
            console.warn("FAILED TO LOAD USER PREFERENCES")
            closeStartupStatus()
            statusPopup.show(getUserDid(), error, QEnums.STATUS_LEVEL_ERROR)
            signOutCurrentUser()
            signIn()
        }

        onTimelineSyncStart: (maxPages, rewindTimestamp) => {
            setStartupRewindStart(maxPages, rewindTimestamp)
        }

        onTimelineSyncProgress: (pages, timestamp) => {
            setStartupRewindProgress(pages, timestamp)
        }

        onTimelineSyncOK: (index, offsetY) => {
            syncTimelineToPost(index, offsetY)
        }

        onTimelineSyncFailed: {
            console.warn("SYNC FAILED")
            syncTimelineToPost(0)
        }

        onGapFilled: (gapEndIndex) => {
            console.debug("Gap filled, end index:", gapEndIndex)
            getTimelineView().moveToPost(gapEndIndex)
        }

        onTimelineResumed: (postIndex, offsetY) => {
            console.debug("Timeline resumed, index:", postIndex, "offsetY:", offsetY)
            getTimelineView().resumeTimeline(postIndex, offsetY)
        }

        onGetDetailedProfileOK: (did, profile) => { // qmllint disable signal-handler-parameters
            Qt.callLater((p) => {
                    let modelId = getSkywalker(did).createAuthorFeedModel(profile)
                    viewAuthor(profile, modelId, did)
                },
                profile)
        }

        onGetFeedGeneratorOK: (did, generatorView, viewPosts) => { // qmllint disable signal-handler-parameters
            if (viewPosts)
                viewPostFeed(generatorView, did)
            else
                viewFeedDescription(generatorView, did)
        }

        onGetStarterPackViewOk: (did, starterPack) => viewStarterPack(starterPack, did) // qmllint disable signal-handler-parameters

        onSharedTextReceived: (text) => {
            closeStartupStatus() // close startup status if sharing started the app                      
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
        onSharedImageReceived: (source, gifTempFileName, text) => {
            closeStartupStatus() // close startup status if sharing started the app

            if (!gifTempFileName) {
                handleSharedImageReceived(source, text)
                return
            }

            guiSettings.askConvertGif(
                rootContent,
                "file://" + gifTempFileName,
                () => gifToVideoConverter.start(gifTempFileName, text),
                () => { postUtils.dropVideo("file://" + gifTempFileName); handleSharedImageReceived(source, text) })
        }

        function handleSharedImageReceived(source, text) {
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

        onSharedVideoReceived: (source, text) => {
            closeStartupStatus() // close startup status if sharing started the app
            handleSharedVideoReceived(source, text)
        }

        function handleSharedVideoReceived(source, text) {
            let item = currentStackItem()

            if (item instanceof ComposePost)
                item.addSharedVideo(source, text)
            else if (item instanceof PostThreadView)
                item.videoReply(text, source)
            else if (item instanceof AuthorView)
                item.mentionVideoPost(text, source)
            else
                composeVideoPost(text, source)
        }

        onSharedDmTextReceived: (text) => {
            closeStartupStatus() // close startup status if sharing started the app
            let item = currentStackItem()

            if (item instanceof MessagesListView)
                item.addMessage(text)
            else
                startConvo(text)
        }

        onShowNotifications: viewNotifications()
        onShowDirectMessages: viewChat()

        onShowLinkReceived: (uri) => {
            console.debug("Got the ShowLinkReceived Signal for:", uri)
            linkUtils.openLink(uri)
        }

        onAnniversary: {
            const years = skywalker.getAnniversary().getAnniversaryYears()
            guiSettings.notice(rootContent,
                qsTr(`Today is your ${years} year Bluesky anniversary. On this day you can send an anniversary card. You can find it on the post page, when you click the button to send a post.`),
                "🥳")
        }

        onUnreadNotificationsLoaded: (mentionsOnly, oldestUnreadIndex) => {
            getNotificationView().moveToNotification(oldestUnreadIndex, mentionsOnly)
        }

        onAppPaused: {
            let current = currentStackItem()

            if (current && typeof current.cover === 'function')
                current.cover()

            getTimelineView().enabled = false
        }

        onAppResumed: {
            // Display mode may have changed while sleeping.
            let userSettings = skywalker.getUserSettings()
            setDisplayMode(userSettings.getDisplayMode())

            let current = currentStackItem()

            if (current && typeof current.uncover === 'function')
                current.uncover()

            getTimelineView().enabled = true
        }

        // Note for search feeds the feedUri is the search name
        function saveLastViewedFeed(feedUri) {
            let userSettings = skywalker.getUserSettings()
            const userDid = skywalker.getUserDid()

            if (feedUri === "home" ||
                    skywalker.favoriteFeeds.isPinnedFeed(feedUri) ||
                    skywalker.favoriteFeeds.isPinnedSearch(feedUri)) {
                userSettings.setLastViewedFeed(userDid, feedUri)
                console.debug("Saved last viewed feed:", feedUri)
            }
        }

        function start() {
            setStartupStatus(qsTr("Loading user profile"))
            skywalker.getUserProfileAndFollows()
        }
    }

    GifToVideoConverter {
        property var progressDialog
        property string postText
        property string gifFileName

        id: gifToVideoConverter

        onConversionOk: (videoFileName) => {
            progressDialog.destroy()
            postUtils.dropVideo("file://" + gifFileName)
            skywalker.handleSharedVideoReceived(`file://${videoFileName}`, postText)
        }

        onConversionFailed: (error) => {
            progressDialog.destroy()
            postUtils.dropVideo("file://" + gifFileName)
            statusPopup.show(skywalker.getUserDid(), qsTr(`GIF conversion failed: ${error}`), QEnums.STATUS_LEVEL_ERROR)
        }

        onConversionProgress: (progress) => progressDialog.setProgress(progress)

        function start(fileName, text) {
            postText = text
            gifFileName = fileName
            progressDialog = guiSettings.showProgress(rootContent, qsTr("Converting GIF to Video"), () => doCancel())
            gifToVideoConverter.convert(fileName)
        }

        function doCancel() {
            gifToVideoConverter.cancel()
            postUtils.dropVideo("file://" + gifFileName)
        }
    }

    SideBar {
        property var favoritesSwipeView: favoritesTabBar.favoritesSwipeView

        id: sideBar
        x: guiSettings.leftMargin
        y: guiSettings.headerMargin
        width: Math.min(parent.width * 0.25, guiSettings.sideBarMaxWidth)
        height: parent.height - guiSettings.headerMargin - guiSettings.footerMargin
        timeline: favoritesSwipeView ? favoritesSwipeView.currentView : null
        skywalker: root.getSkywalker()
        homeActive: rootContent.currentIndex === rootContent.timelineIndex
        notificationsActive: rootContent.currentIndex === rootContent.notificationIndex
        searchActive: rootContent.currentIndex === rootContent.searchIndex
        messagesActive: rootContent.currentIndex === rootContent.chatIndex
        onHomeClicked: {
            if (homeActive)
                favoritesSwipeView.currentView.moveToHome()
            else
                root.viewTimeline()
        }
        onNotificationsClicked: {
            if (!notificationsActive)
                viewNotifications()
            else if (currentStackItem() instanceof NotificationListView)
                currentStackItem().handleNotificationsClicked()
        }
        onSearchClicked: {
            if (!searchActive)
                viewSearchView()
        }
        onMessagesClicked: {
            if (!messagesActive)
                viewChat()
            else if (currentStackItem() instanceof ConvoListView)
                currentStackItem().positionViewAtBeginning()
        }
        onAddConvoClicked: {
            if (currentStackItem() instanceof ConvoListView)
                currentStackItem().addConvo()
        }

        visible: showSideBar && currentStackItem() && typeof currentStackItem().noSideBar === 'undefined'
    }

    StackLayout {
        readonly property int timelineIndex: 0
        readonly property int notificationIndex: 1
        readonly property int searchIndex: 2
        readonly property int chatIndex: 3
        property int prevIndex: timelineIndex

        id: rootContent
        x: sideBar.visible ? sideBar.x + sideBar.width : 0
        width: parent.width - x - guiSettings.rightMargin
        height: parent.height
        currentIndex: timelineIndex
        clip: true

        onCurrentIndexChanged: {
            let prevStack = rootContent.children[prevIndex]

            if (prevStack.depth > 0) {
                let prevItem = prevStack.get(prevStack.depth - 1)

                if (typeof prevItem.cover === 'function')
                    prevItem.cover()
            }

            if (prevIndex === notificationIndex) {
                skywalker.notificationListModel.updateRead()
                skywalker.mentionListModel.updateRead()
                getNotificationView().reset()
                unwindStack(notificationStack)
            }

            prevIndex = currentIndex
            let currentItem = currentStackItem()

            if (currentItem && typeof currentItem.uncover === 'function')
                currentItem.uncover()

            favoritesTabBar.update()
        }

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
            id: chatStack
        }
    }

    // Right margin (navbar on Android)
    Rectangle {
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: rootContent.right
        anchors.right: parent.right
        color: getBackgroundColor()

        function getBackgroundColor() {
            const item = currentStackItem()

            if (item && typeof item.background !== 'undefined' && item.background !== null && typeof item.background.color !== 'undefined')
                return item.background.color

            if (item && typeof item.color !== 'undefined')
                return item.color

            return guiSettings.backgroundColor
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

    // Color the status bar on Android for edge-to-edge mode
    Rectangle {
        width: parent.width
        height: guiSettings.headerMargin
        z: guiSettings.headerZLevel
        color: displayUtils.statusBarColor
    }

    SettingsDrawer {
        id: settingsDrawer
        height: parent.height
        edge: !showSideBar ? Qt.RightEdge : Qt.LeftEdge
        dragMargin: 0
        modal: true

        onAboutToShow: enablePopupShield(true)
        onAboutToHide: enablePopupShield(false)

        onProfile: {
            let did = skywalker.getUserDid()
            skywalker.getDetailedProfile(did)
            close()
        }

        // onInviteCodes: {
        //     let component = guiSettings.createComponent("InviteCodesView.qml")
        //     const codes = inviteCodeStore.getCodes()
        //     const failedToLoad = inviteCodeStore.failedToLoad()
        //     let page = component.createObject(root, { codes: codes, failedToLoad: failedToLoad })
        //     page.onClosed.connect(() => { popStack() })
        //     page.onAuthorClicked.connect((did) => { skywalker.getDetailedProfile(did) })
        //     pushStack(page)
        //     close()
        // }

        onBookmarks: {
            let component = guiSettings.createComponent("Bookmarks.qml")
            let page = component.createObject(root, { skywalker: skywalker })
            page.onClosed.connect(() => { popStack() })
            pushStack(page)
            skywalker.getBookmarks().getBookmarks()
            close()
        }

        onContentFiltering: {
            editContentFilterSettings()
            close()
        }

        onActiveFollows: {
            let userSettings = skywalker.getUserSettings()
            const interval = userSettings.getActiveOnlineIntervalMins()
            let modelId = skywalker.createAuthorListModel(QEnums.AUTHOR_LIST_ACTIVE_FOLLOWS, "")
            viewAuthorList(modelId, qsTr("Now Online"),
                    qsTr(`Users you follow that have been active in the last ${interval} minutes.`),
                    false)
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
            let component = guiSettings.createComponent("MutedWords.qml")
            let page = component.createObject(root, { skywalker: skywalker })
            page.onClosed.connect(() => { popStack() })
            pushStack(page)
            close()
        }

        onFocusHashtags: {
            let component = guiSettings.createComponent("FocusHashtags.qml")
            let page = component.createObject(root)
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
            skywalker.deleteSession()
            close()
        }

        onAbout: {
            showAbout()
            close()
        }

        onBuyCoffee: {
            linkUtils.openLink("https://buymeacoffee.com/skywalker.thereforeiam.eu")
            close()
        }

        function show() {
            user = skywalker.getUser()
            open()
        }
    }

    SwitchUserDrawer {
        id: switchUserDrawer
        width: parent.width
        height: parent.height * 0.7
        edge: Qt.BottomEdge
        dragMargin: 0
        bottomPadding: guiSettings.footerMargin
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
                skywalker.switchUser(profile.did)

                if (skywalker.resumeAndRefreshSession()) {
                    showStartupStatus()
                }
                else if (skywalker.autoLogin()) {
                    showStartupStatus()
                }
                else {
                    const userSettings = skywalker.getUserSettings()
                    const host = userSettings.getHost(profile.did)
                    loginUser(host, profile.handle, profile.did)
                }
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
        property string repostByDid
        property string repostedAlreadyUri
        property string repostUri
        property string repostCid
        property string repostViaUri
        property string repostViaCid
        property string repostText
        property date repostDateTime
        property basicprofile repostAuthor
        property bool repostEmbeddingDisabled
        property string repostPlainText

        id: repostDrawer
        width: parent.width
        edge: Qt.BottomEdge
        dragMargin: 0
        bottomPadding: guiSettings.footerMargin
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
                    svg: SvgOutline.close
                    accessibleName: qsTr("cancel repost")
                    onClicked: repostDrawer.close()
                }

                SkyButton {
                    id: repostButton
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: repostDrawer.repostedAlreadyUri ? qsTr("Undo repost") : qsTr("Repost")
                    onClicked: {
                        if (repostDrawer.repostedAlreadyUri)
                            getPostUtils(repostDrawer.repostByDid).undoRepost(repostDrawer.repostedAlreadyUri, repostDrawer.repostCid)
                        else
                            getPostUtils(repostDrawer.repostByDid).repost(repostDrawer.repostUri, repostDrawer.repostCid, repostDrawer.repostViaUri, repostDrawer.repostViaCid)

                        repostDrawer.close()
                    }
                }
            }
            SkyButton {
                id: quotePostButton
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Quote post")
                enabled: !repostDrawer.repostEmbeddingDisabled
                onClicked: {
                    // No need to check if post still exist. Already checked before
                    // opening this drawer
                    root.doComposeQuote(repostDrawer.repostUri, repostDrawer.repostCid,
                                      repostDrawer.repostText, repostDrawer.repostDateTime,
                                      repostDrawer.repostAuthor, "", repostDrawer.repostByDid)
                    repostDrawer.close()
                }
            }

            SkyButton {
                id: copyQuotePostButton
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Copy & quote post")
                enabled: !repostDrawer.repostEmbeddingDisabled
                onClicked: {
                    // No need to check if post still exist. Already checked before
                    // opening this drawer
                    root.doComposeQuote(repostDrawer.repostUri, repostDrawer.repostCid,
                                      repostDrawer.repostText, repostDrawer.repostDateTime,
                                      repostDrawer.repostAuthor, repostDrawer.repostPlainText,
                                      repostDrawer.repostByDid)
                    repostDrawer.close()
                }
            }

            SkyButton {
                id: quoteInMessageButton
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Quote in direct message")
                enabled: !repostDrawer.repostEmbeddingDisabled
                visible: isActiveUser(repostDrawer.repostByDid)
                onClicked: {
                    const link = linkUtils.toHttpsLink(repostDrawer.repostUri)
                    startConvo(link)
                    repostDrawer.close()
                }
            }
        }

        function show(hasRepostedUri, uri, cid, viaUri, viaCid, text, dateTime, author, embeddingDisabled, plainText, byDid = "") {
            repostByDid = byDid
            repostedAlreadyUri =  hasRepostedUri
            repostUri = uri
            repostCid = cid
            repostViaUri = viaUri
            repostViaCid = viaCid
            repostText = text
            repostDateTime = dateTime
            repostAuthor = author
            repostEmbeddingDisabled = embeddingDisabled
            repostPlainText = plainText
            open()
        }
    }

    MainPostUtils {
        id: postUtils
        skywalker: skywalker
    }

    MainProfileUtils {
        id: profileUtils
        skywalker: skywalker
    }

    MainFeedUtils {
        id: feedUtils
        skywalker: skywalker
    }

    MainLinkUtils {
        id: linkUtils
        skywalker: skywalker // qmllint disable missing-type
    }

    MainGraphUtils {
        id: graphUtils
        skywalker: skywalker
    }

    M3U8Reader {
        id: m3u8Reader
        videoQuality: skywalker.getUserSettings().videoQuality

        onGetVideoStreamOk: (durationMs) => {
            const fileName = videoUtils.getVideoFileNameForGallery("ts")

            if (!fileName) {
                skywalker.showStatusMessage(qsTr("Cannot create gallery file"), QEnums.STATUS_LEVEL_ERROR)
                return
            }

            loadStream(fileName)
        }

        // NOTE: Transcoding to MP4 results in an MP4 that cannot be played by the gallery
        // app. The MP4 can be played with other players. For now save as mpeg-ts. This
        // can be played by the gallery app.
        onGetVideoStreamError: skywalker.showStatusMessage(qsTr("Failed to save video"), QEnums.STATUS_LEVEL_ERROR)

        onLoadStreamOk: (videoSource) => {
            videoUtils.indexGalleryFile(videoSource.slice(7))
            skywalker.showStatusMessage(qsTr("Video saved"), QEnums.STATUS_LEVEL_INFO)
        }

        onLoadStreamError: skywalker.showStatusMessage(qsTr("Failed to save video"), QEnums.STATUS_LEVEL_ERROR)
    }

    VideoUtils {
        id: videoUtils

        onCopyVideoOk: skywalker.showStatusMessage(qsTr("Video saved"), QEnums.STATUS_LEVEL_INFO)
        onCopyVideoFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
    }

    DisplayUtils {
        id: displayUtils
        skywalker: skywalker
    }

    Utils {
        id: utils
        skywalker: skywalker
    }

    // InviteCodeStore {
    //     id: inviteCodeStore
    //     skywalker: skywalker

    //     onLoaded: skywalker.notificationListModel.addInviteCodeUsageNofications(inviteCodeStore)
    // }

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

    function openLink(link, containingText, openByDid = "") {
        if (link.startsWith("@")) {
            console.debug("@-MENTION:", link)
            getSkywalker(openByDid).getDetailedProfile(link.slice(1))
        } else if (link.startsWith("did:")) {
            console.debug("DID-MENTION:", link)
            getSkywalker(openByDid).getDetailedProfile(link)
        } else if (UnicodeFonts.isHashtag(link)) {
            console.debug("#-TAG:", link)
            viewSearchView(link)
        } else {
            getLinkUtils(openByDid).openLink(link, containingText)
        }
    }

    function showStartupStatus() {
        let component = guiSettings.createComponent("StartupStatus.qml")
        let page = component.createObject(root)
        page.setStatus(qsTr("Connecting"))
        pushStack(page, StackView.Immediate)
    }

    function setStartupStatus(msg) {
        let item = currentStackItem()

        if (item instanceof StartupStatus)
            item.setStatus(msg)
    }

    function setStartupRewindStart(pages, timestamp) {
        let item = currentStackItem()

        if (item instanceof StartupStatus)
            item.startRewind(pages, timestamp)
    }

    function setStartupRewindProgress(pages, timestamp) {
        let item = currentStackItem()

        if (item instanceof StartupStatus)
            item.updateRewindProgress(pages, timestamp)
    }

    function closeStartupStatus() {
        let item = currentStackItem()

        if (item instanceof StartupStatus)
            popStack()
    }

    function showAbout() {
        let component = guiSettings.createComponent("About.qml")
        let page = component.createObject(root)
        page.onClosed.connect(() => { popStack() })
        pushStack(page)
    }

    function signIn() {
        let component = guiSettings.createComponent("SignIn.qml")
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

    function loginUser(host, handle, did, error="", msg="", password="") {
        console.debug("login, host:", host, "handle:", handle, "did:", did)
        let component = guiSettings.createComponent("Login.qml")
        let page = component.createObject(root, {
                host: host,
                user: handle,
                did: did,
                errorCode: error,
                errorMsg: msg,
                password: password
        })
        page.onCanceled.connect(() => {
                popStack()
                signIn()
        })
        page.onAccepted.connect((host, handle, password, did, rememberPassword, authFactorToken,
                                 setAdvancedSettings, serviceAppView, serviceChat, serviceVideoHost, serviceVideoDid) => {
                popStack()
                const user = did ? did : handle
                skywalkerLogin(host, user, password, rememberPassword, authFactorToken,
                               setAdvancedSettings, serviceAppView, serviceChat, serviceVideoHost, serviceVideoDid)
        })
        pushStack(page)
    }

    function newUser() {
        let component = guiSettings.createComponent("Login.qml")
        let page = component.createObject(root)
        page.onCanceled.connect(() => {
                popStack()
                signIn()
        })
        page.onAccepted.connect((host, handle, password, did, rememberPassword, _authFactorToken,
                                 setAdvancedSettings, serviceAppView, serviceChat, serviceVideoHost, serviceVideoDid) => {
                popStack()
                skywalkerLogin(host, handle, password, rememberPassword, "",
                               setAdvancedSettings, serviceAppView, serviceChat, serviceVideoHost, serviceVideoDid)
        })

        pushStack(page)
    }

    function selectUser() {
        const userSettings = skywalker.getUserSettings()
        const userList = userSettings.getUserListWithAddAccount()

        let component = guiSettings.createComponent("SelectSignInUser.qml")
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

                skywalker.switchUser(profile.did)

                if (skywalker.resumeAndRefreshSession()) {
                    showStartupStatus()
                }
                else {
                    if (userSettings.getRememberPassword(profile.did)) {
                        const password = userSettings.getPassword(profile.did)
                        skywalkerLogin("", profile.did, password, true)
                    }
                    else {
                        const host = userSettings.getHost(profile.did)
                        loginUser(host, profile.handle, profile.did)
                    }
                }
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

    function skywalkerLogin(host, user, password, rememberPassword, authFactorToken,
                            setAdvancedSettings, serviceAppView, serviceChat, serviceVideoHost, serviceVideoDid) {
        showStartupStatus()
        skywalker.login(host, user, password, rememberPassword, authFactorToken,
                        setAdvancedSettings, serviceAppView, serviceChat, serviceVideoHost, serviceVideoDid)
    }

    function signOutCurrentUser() {
        skywalker.stopTimelineAutoUpdate()
        getTimelineView().stopSync()
        getFavoritesSwipeView().reset()
        unwindStack()
        destroySearchView()
        // inviteCodeStore.clear()
        skywalker.signOut()
    }

    function composePost(initialText = "", imageSource = "", postByDid = "") {
        let component = guiSettings.createComponent("ComposePost.qml")
        let page = component.createObject(root, {
                postByDid: postByDid,
                initialText: initialText,
                initialImage: imageSource
        })
        page.onClosed.connect(() => { popStack() })
        pushStack(page)
    }

    function composeVideoPost(initialText = "", videoSource = "", postByDid = "") {
        let component = guiSettings.createComponent("ComposePost.qml")
        let page = component.createObject(root, {
                postByDid: postByDid,
                initialText: initialText,
                initialVideo: videoSource
        })
        page.onClosed.connect(() => { popStack() })
        pushStack(page)
    }

    function composeReply(replyToUri, replyToCid, replyToText, replyToDateTime, replyToAuthor,
                          replyRootUri, replyRootCid, replyToLanguage, replyToMentionDids, initialText = "", imageSource = "",
                          postByDid = "")
    {
        const pu = getPostUtils(postByDid)
        pu.checkPost(replyToUri, replyToCid,
            () => doComposeReply(replyToUri, replyToCid, replyToText, replyToDateTime, replyToAuthor,
                                 replyRootUri, replyRootCid, replyToLanguage, replyToMentionDids,initialText, imageSource,
                                 postByDid))
    }

    function doComposeReply(replyToUri, replyToCid, replyToText, replyToDateTime, replyToAuthor,
                          replyRootUri, replyRootCid, replyToLanguage, replyToMentionDids, initialText = "", imageSource = "",
                          postByDid = "")
    {
        let component = guiSettings.createComponent("ComposePost.qml")
        let page = component.createObject(root, {
                postByDid: postByDid,
                initialText: initialText,
                initialImage: imageSource,
                replyToPostUri: replyToUri,
                replyToPostCid: replyToCid,
                replyRootPostUri: replyRootUri,
                replyRootPostCid: replyRootCid,
                replyToPostText: replyToText,
                replyToPostDateTime: replyToDateTime,
                replyToAuthor: replyToAuthor,
                replyToLanguage: replyToLanguage,
                replyToMentionDids: replyToMentionDids
        })
        page.onClosed.connect(() => { popStack() })
        pushStack(page)
    }

    function composeVideoReply(replyToUri, replyToCid, replyToText, replyToDateTime, replyToAuthor,
                               replyRootUri, replyRootCid, replyToLanguage, replyToMentionDids,
                               initialText, videoSource, postByDid = "")
    {
        const pu = getPostUtils(postByDid)
        pu.checkPost(replyToUri, replyToCid,
            () => doComposeVideoReply(replyToUri, replyToCid, replyToText, replyToDateTime, replyToAuthor,
                                      replyRootUri, replyRootCid, replyToLanguage, replyToMentionDids,
                                      initialText, videoSource, postByDid))
    }

    function doComposeVideoReply(replyToUri, replyToCid, replyToText, replyToDateTime, replyToAuthor,
                                 replyRootUri, replyRootCid, replyToLanguage, replyToMentionDids,
                                 initialText, videoSource, postByDid = "")
    {
        let component = guiSettings.createComponent("ComposePost.qml")
        let page = component.createObject(root, {
                postByDid: postByDid,
                initialText: initialText,
                initialVideo: videoSource,
                replyToPostUri: replyToUri,
                replyToPostCid: replyToCid,
                replyRootPostUri: replyRootUri,
                replyRootPostCid: replyRootCid,
                replyToPostText: replyToText,
                replyToPostDateTime: replyToDateTime,
                replyToAuthor: replyToAuthor,
                replyToLanguage: replyToLanguage,
                replyToMentionDids: replyToMentionDids
        })
        page.onClosed.connect(() => { popStack() })
        pushStack(page)
    }

    function doComposeQuote(quoteUri, quoteCid, quoteText, quoteDateTime, quoteAuthor,
                            initialText = "", quoteByDid = "")
    {
        let component = guiSettings.createComponent("ComposePost.qml")
        let page = component.createObject(root, {
                postByDid: quoteByDid,
                initialText: initialText,
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

    function repost(repostUri, uri, cid, viaUri, viaCid, text, dateTime, author, embeddingDisabled, plainText, repostByDid = "") {
        const pu = getPostUtils(repostByDid)
        pu.checkPost(uri, cid,
            () => doRepost(repostUri, uri, cid, viaUri, viaCid, text, dateTime, author, embeddingDisabled, plainText, repostByDid))
    }

    function doRepost(repostUri, uri, cid, viaUri, viaCid, text, dateTime, author, embeddingDisabled, plainText, repostByDid = "") {
        repostDrawer.show(repostUri, uri, cid, viaUri, viaCid, text, dateTime, author, embeddingDisabled, plainText, repostByDid)
    }

    function quotePost(uri, cid, text, dateTime, author, embeddingDisabled, quoteByDid = "") {
        if (embeddingDisabled) {
            skywalker.showStatusMessage(qsTr("Quoting not allowed"), QEnums.STATUS_LEVEL_INFO)
            return
        }

        const pu = getPostUtils(quoteByDid)
        pu.checkPost(uri, cid, () => doComposeQuote(uri, cid, text, dateTime, author, "", quoteByDid))
    }

    function like(likeUri, uri, cid, viaUri = "", viaCid = "", likeByDid = "") {
        const pu = getPostUtils(likeByDid)

        if (!pu)
            return

        if (likeUri)
            pu.undoLike(likeUri, cid)
        else
            pu.like(uri, cid, viaUri, viaCid)
    }

    function likeFeed(likeUri, uri, cid, likeByDid = "") {
        const fu = getFeedUtils(likeByDid)

        if (likeUri)
            fu.undoLike(likeUri, cid)
        else
            fu.like(uri, cid)
    }

    function likeByNonActiveUser(mouseEvent, mouseView, parentView, postUri, viaUri, viaCid) {
        return actionByNonActiveUser(mouseEvent, mouseView, parentView, postUri,
                              QEnums.NON_ACTIVE_USER_LIKE,
                              (user) => { user.like(viaUri, viaCid) })
    }

    function bookmarkByNonActiveUser(mouseEvent, mouseView, parentView, postUri) {
        return actionByNonActiveUser(mouseEvent, mouseView, parentView, postUri,
                              QEnums.NON_ACTIVE_USER_BOOKMARK,
                              (user) => { user.bookmark() })
    }

    function replyByNonActiveUser(mouseEvent, mouseView, parentView,
                                  replyToUri, replyToCid, replyToText, replyToDateTime, replyToAuthor,
                                  replyRootUri, replyRootCid, replyToLanguage, replyToMentionDids) {
        return actionByNonActiveUser(
                    mouseEvent, mouseView, parentView, replyToUri,
                    QEnums.NON_ACTIVE_USER_REPLY,
                    (user) => {
                        if (user.postView.replyDisabled) {
                            console.warn("Reply disabled:", replyToUri)
                            return
                        }

                        composeReply(replyToUri, replyToCid, replyToText, replyToDateTime, replyToAuthor,
                                     replyRootUri, replyRootCid, replyToLanguage, replyToMentionDids,
                                     "", "", user.profile.did)
                    })
    }

    function repostByNonActiveUser(mouseEvent, mouseView, parentView, postUri, postCid,
                                   text, dateTime, author, embeddingDisabled, viaUri = "", viaCid = "") {
        return actionByNonActiveUser(
                    mouseEvent, mouseView, parentView, postUri,
                    QEnums.NON_ACTIVE_USER_REPOST,
                    (user) => { // fist action: repost
                        user.repost(viaUri, viaCid)
                    },
                    (user) => { // second action: quote post
                        if (embeddingDisabled) {
                            console.warn("Embedding disabled:", postUri)
                            return
                        }

                        quotePost(postUri, postCid, text, dateTime, author, embeddingDisabled,
                                  user.profile.did)
                    })
    }

    function actionByNonActiveUser(mouseEvent, mouseView, parentView, postUri, actionType, actionCb, secondActionCb) {
        if (!skywalker.getSessionManager().hasNonActiveUsers()) {
            console.debug("No non-active users")
            return false
        }

        const mousePoint = mouseView.mapToItem(parentView, 0, mouseEvent.y)
        let component = guiSettings.createComponent("NonActiveUsersPopup.qml")
        let popup = component.createObject(parentView, {
                mouseY: mousePoint.y,
                postUri: postUri,
                action: actionType
            })
        popup.onUserClicked.connect((user) => {
                if (actionType === QEnums.NON_ACTIVE_USER_REPLY)
                    popup.destroy()

                actionCb(user)
            })
        popup.onRepostClicked.connect((user) => {
                actionCb(user)
            })
        popup.onQuoteClicked.connect((user) => {
                popup.destroy()
                secondActionCb(user)
            })
        popup.onClosed.connect(() => { popup.destroy() })
        popup.open()

        return true
    }

    function isActiveUser(did) {
        return !did || skywalker.getUserDid() === did
    }

    function showMoreLikeThis(feedDid, postUri, feedContext, interactByDid = "") {
        const fu = getFeedUtils(interactByDid)
        fu.showMoreLikeThis(postUri, feedDid, feedContext)
    }

    function showLessLikeThis(feedDid, postUri, feedContext, interactByDid = "") {
        const fu = getFeedUtils(interactByDid)
        fu.showLessLikeThis(postUri, feedDid, feedContext)
    }

    function muteThread(uri, threadMuted, muteByDid = "") {
        if (!uri) {
            console.warn("Cannot mute thread, empty uri")
            return
        }

        const pu = getPostUtils(muteByDid)

        if (threadMuted)
            pu.unmuteThread(uri)
        else
            pu.muteThread(uri)
    }

    function detachQuote(uri, embeddingUri, embeddingCid, detach, detachByDid = "") {
        const pu = getPostUtils(detachByDid)
        pu.detachQuote(uri, embeddingUri, embeddingCid, detach)
    }

    function hidePostReply(threadgateUri, rootUri, rootCid, uri, replyRestriction, replyRestrictionLists, postHiddenReplies, hideByDid = "") {
        let hidden = postHiddenReplies
        const index = hidden.indexOf(uri)

        if (index < 0) {
            guiSettings.askYesNoQuestion(rootContent,
                qsTr("Do you want to move this reply to the hidden section at the bottom of your thread (in next post thread views), and mute notifications both for yourself and others?"),
                () => {
                    hidden.push(uri)
                    hidePostReplyContinue(threadgateUri, rootUri, rootCid, uri, replyRestriction, replyRestrictionLists, hidden, hideByDid)
                })
        }
        else {
            hidden.splice(index, 1)
            hidePostReplyContinue(threadgateUri, rootUri, rootCid, uri, replyRestriction, replyRestrictionLists, hidden, hideByDid)
        }
    }

    function hidePostReplyContinue(threadgateUri, rootUri, rootCid, uri, replyRestriction, replyRestrictionLists, postHiddenReplies, hideByDid = "") {
        const allowMentioned = Boolean(replyRestriction & QEnums.REPLY_RESTRICTION_MENTIONED)
        const allowFollower = Boolean(replyRestriction & QEnums.REPLY_RESTRICTION_FOLLOWER)
        const allowFollowing = Boolean(replyRestriction & QEnums.REPLY_RESTRICTION_FOLLOWING)
        const allowNobody = Boolean(replyRestriction === QEnums.REPLY_RESTRICTION_NOBODY)
        const pu = getPostUtils(hideByDid)

        if (postHiddenReplies.length === 0 && threadgateUri && replyRestriction === QEnums.REPLY_RESTRICTION_NONE) {
            pu.undoThreadgate(threadgateUri, rootCid)
        }
        else {
            pu.addThreadgate(rootUri, rootCid, allowMentioned, allowFollower, allowFollowing,
                                    replyRestrictionLists, allowNobody, postHiddenReplies)
        }
    }

    function gateRestrictions(threadgateUri, rootUri, rootCid, uri, replyRestriction, replyRestrictionLists, postHiddenReplies, restrictByDid = "") {
        const sw = getSkywalker(restrictByDid)
        const pu = getPostUtils(restrictByDid)

        const restrictionsListModelId = sw.createListListModel(QEnums.LIST_TYPE_ALL, QEnums.LIST_PURPOSE_CURATE, sw.getUserDid())
        sw.getListList(restrictionsListModelId)
        pu.setAllowListUris(replyRestrictionLists)

        let component = guiSettings.createComponent("AddReplyRestrictions.qml")
        let restrictionsPage = component.createObject(currentStackItem(), {
                userDid: restrictByDid,
                rootUri: rootUri,
                postUri: uri,
                restrictReply: replyRestriction !== QEnums.REPLY_RESTRICTION_NONE,
                allowMentioned: replyRestriction & QEnums.REPLY_RESTRICTION_MENTIONED,
                allowFollower: replyRestriction & QEnums.REPLY_RESTRICTION_FOLLOWER,
                allowFollowing: replyRestriction & QEnums.REPLY_RESTRICTION_FOLLOWING,
                allowLists: pu.allowLists,
                allowListIndexes: pu.allowListIndexes,
                allowListUrisFromDraft: pu.allowListUris,
                listModelId: restrictionsListModelId
        })
        restrictionsPage.onAccepted.connect(() => {
                if (restrictionsPage.restrictReply === Boolean(replyRestriction !== QEnums.REPLY_RESTRICTION_NONE) &&
                    restrictionsPage.allowMentioned === Boolean(replyRestriction & QEnums.REPLY_RESTRICTION_MENTIONED) &&
                    restrictionsPage.allowFollower === Boolean(replyRestriction & QEnums.REPLY_RESTRICTION_FOLLOWER) &&
                    restrictionsPage.allowFollowing === Boolean(replyRestriction & QEnums.REPLY_RESTRICTION_FOLLOWING) &&
                    !checkRestrictionListsChanged(getReplyRestrictionListUris(restrictionsListModelId, restrictionsPage.allowLists, restrictionsPage.allowListIndexes, restrictByDid), pu))
                {
                    console.debug("No reply restriction change!")
                }
                else if (threadgateUri && !restrictionsPage.restrictReply && postHiddenReplies.length === 0) {
                    pu.undoThreadgate(threadgateUri, rootCid)
                }
                else {
                    const allowLists = getReplyRestrictionLists(restrictionsListModelId, restrictionsPage.allowLists, restrictionsPage.allowListIndexes, restrictByDid)
                    const allowNobody = Boolean(restrictionsPage.restrictReply && !restrictionsPage.allowMentioned && !restrictionsPage.allowFollower && !restrictionsPage.allowFollowing && (allowLists.length === 0))

                    pu.addThreadgate(rootUri, rootCid,
                                            restrictionsPage.allowMentioned,
                                            restrictionsPage.allowFollower,
                                            restrictionsPage.allowFollowing,
                                            allowLists, allowNobody, postHiddenReplies)
                }

                if (restrictionsPage.prevAllowQuoting !== restrictionsPage.allowQuoting) {
                    const detachedUris = restrictionsPage.postgate.detachedEmbeddingUris

                    if (!restrictionsPage.postgate.isNull() && detachedUris.length === 0 && restrictionsPage.allowQuoting)
                        pu.undoPostgate(uri)
                    else
                        pu.addPostgate(uri, !restrictionsPage.allowQuoting, detachedUris)
                }
                else {
                    console.debug("No postgate change")
                }

                restrictionsPage.destroy()
                sw.removeListListModel(restrictionsListModelId)
        })
        restrictionsPage.onRejected.connect(() => {
                restrictionsPage.destroy()
                sw.removeListListModel(restrictionsListModelId)
        })
        restrictionsPage.open()
    }

    function checkRestrictionListsChanged(uris, pu) {
        if (uris.length !== pu.allowListUris.length)
            return true

        for (let i = 0; i < uris.length; ++i) {
            if (uris[i] !== pu.allowListUris[i])
                return true
        }

        return false
    }

    function getReplyRestrictionLists(restrictionsListModelId, allowLists, allowListIndexes, restrictByDid = "") {
        const sw = getSkywalker(restrictByDid)
        const pu = getPostUtils(restrictByDid)
        pu.allowListViews = []

        for (let i = 0; i < allowLists.length; ++i) {
            if (allowLists[i]) {
                let model = sw.getListListModel(restrictionsListModelId)
                const listView = model.getEntry(allowListIndexes[i])
                pu.allowListViews.push(listView)
            }
        }

        return pu.allowListViews
    }

    function getReplyRestrictionListUris(restrictionsListModelId, allowLists, allowListIndexes, restrictByDid = "") {
        const sw = getSkywalker(restrictByDid)
        let uris = []

        for (let i = 0; i < allowLists.length; ++i) {
            if (allowLists[i]) {
                let model = sw.getListListModel(restrictionsListModelId)
                const listView = model.getEntry(allowListIndexes[i])
                uris.push(listView.uri)
            }
        }

        console.debug("Restriction lists:", uris)
        return uris
    }

    function deletePost(uri, cid, deleteByDid = "") {
        const pu = getPostUtils(deleteByDid)
        pu.deletePost(uri, cid)
    }

    function pinPost(uri, cid, pinByDid = "") {
        const sw = getSkywalker(pinByDid)
        const profUtils = getProfileUtils(pinByDid)
        const did = sw.getUserDid()
        profUtils.setPinnedPost(did, uri, cid)
    }

    function unpinPost(cid, pinByDid = "") {
        const sw = getSkywalker(pinByDid)
        const profUtils = getProfileUtils(pinByDid)
        const did = sw.getUserDid()
        profUtils.clearPinnedPost(did, cid)
    }

    function showBlockMuteDialog(blockUser, author, okCb, blockByDid = "") {
        let component = guiSettings.createComponent("BlockMuteUser.qml")
        let dialog = component.createObject(root, { blockUser: blockUser, author: author })
        dialog.onAccepted.connect(() => {
            const today = new Date()

            if (dialog.expiresAt <= today)
                root.getSkywalker(blockByDid).showStatusMessage(qsTr("Already expired"), QEnums.STATUS_LEVEL_ERROR)
            else
                okCb(dialog.expiresAt)

            dialog.destroy()
        })
        dialog.onRejected.connect(() => dialog.destroy())
        dialog.open()
    }

    function blockAuthor(author, blockByDid = "") {
        let gu = getGraphUtils(blockByDid)
        let did = author.did
        showBlockMuteDialog(true, author, (expiresAt) => gu.block(did, expiresAt), blockByDid)
    }

    function viewPostThread(did, modelId, postEntryIndex) {
        console.debug("View post thread:", did)
        let component = guiSettings.createComponent("PostThreadView.qml")
        let view = component.createObject(root, {
                userDid: did,
                modelId: modelId,
                postEntryIndex: postEntryIndex })
        view.onClosed.connect(() => { popStack() }) // qmllint disable missing-property
        pushStack(view)
    }

    function savePhoto(sourceUrl) {
        postUtils.savePhoto(sourceUrl)
    }

    function sharePhotoToApp(sourceUrl) {
        postUtils.sharePhotoToApp(sourceUrl)
    }

    function viewFullImage(imageList, currentIndex, previewImage, closeCb) {
        let component = guiSettings.createComponent("FullImageView.qml")
        let view = component.createObject(root, {
                images: imageList,
                imageIndex: currentIndex,
                previewImage: previewImage,
                closeCb: closeCb
        })
        view.onClosed.connect(() => { popStack(null, StackView.Immediate) }) // qmllint disable missing-property
        view.onSaveImage.connect((sourceUrl) => { savePhoto(sourceUrl) })
        view.onShareImage.connect((sourceUrl) => { sharePhotoToApp(sourceUrl) })
        pushStack(view, StackView.Immediate)
    }

    function viewFullAnimatedImage(imageUrl, imageTitle) {
        let component = guiSettings.createComponent("FullAnimatedImageView.qml")
        let view = component.createObject(root, { imageUrl: imageUrl, imageTitle: imageTitle })
        view.onClosed.connect(() => { popStack() }) // qmllint disable missing-property
        pushStack(view)
    }

    function saveVideo(videoSource, playlistUrl) {
        if (videoSource && videoSource.startsWith("file://")) {
            videoUtils.copyVideoToGallery(videoSource.slice(7))
        }
        else if (playlistUrl.endsWith(".m3u8")) {
            m3u8Reader.getVideoStream(playlistUrl)
            skywalker.showStatusMessage(qsTr("Saving video"), QEnums.STATUS_LEVEL_INFO, 60)
        }
        else {
            skywalker.showStatusMessage(qsTr(`Cannot save: ${videoView.playlistUrl}`), QEnums.STATUS_LEVEL_ERROR)
        }
    }

    function viewFullVideo(videoView) {
        let component = guiSettings.createComponent("FullVideoView.qml")
        let view = component.createObject(root, { videoView: videoView })
        view.onClosed.connect(() => { popStack() }) // qmllint disable missing-property
        pushStack(view)
    }

    function viewTimeline() {
        rootContent.currentIndex = rootContent.timelineIndex
    }

    function viewNotifications() {
        rootContent.currentIndex = rootContent.notificationIndex
        const unread = skywalker.getSessionManager().activeUserUnreadNotificationCount

        if (unread > 0) {
            const loadCount = Math.min(100, Math.max(10, unread))
            skywalker.getNotifications(loadCount, true, false, true)
            skywalker.getNotifications(loadCount, false, true, true)

            let view = getNotificationView()
            view.showOwnNotificationsTab()
        }
        else {
            if (skywalker.notificationListModel.rowCount() === 0)
                skywalker.getNotifications(10, false, false)

            if (skywalker.mentionListModel.rowCount() === 0)
                skywalker.getNotifications(10, false, true)

            let view = getNotificationView()
            view.showFirstTabWithUnreadNotifications()
        }
    }

    function viewChat() {
        rootContent.currentIndex = rootContent.chatIndex

        if (!skywalker.chat.convosLoaded(QEnums.CONVO_STATUS_REQUEST))
            skywalker.chat.getConvos(QEnums.CONVO_STATUS_REQUEST)

        if (!skywalker.chat.convosLoaded(QEnums.CONVO_STATUS_ACCEPTED))
            skywalker.chat.getConvos(QEnums.CONVO_STATUS_ACCEPTED)
    }

    function startConvo(text) {
        viewChat()
        getChatView().addConvo(text)
    }

    function createSearchView() {
        let searchComponent = guiSettings.createComponent("SearchView.qml")
        let searchView = searchComponent.createObject(root,
                { skywalker: skywalker, timeline: getTimelineView(), })
        searchView.onClosed.connect(() => { rootContent.currentIndex = rootContent.timelineIndex })
        searchStack.push(searchView)
    }

    function destroySearchView() {
        if (searchStack.depth > 0) {
            let item = searchStack.get(0)
            item.forceDestroy()
            searchStack.clear()
        }
    }

    function viewSearchView(searchText = "", searchScope = "") {
        rootContent.currentIndex = rootContent.searchIndex

        if (searchStack.depth === 0)
            createSearchView()

        unwindStack()
        currentStackItem().show(searchText, searchScope)
    }

    function viewSearchViewFeed(searchFeed) {
        rootContent.currentIndex = rootContent.searchIndex

        if (searchStack.depth === 0)
            createSearchView()

        unwindStack()
        currentStackItem().showSearchFeed(searchFeed)
    }

    function viewHomeFeed() {
        viewTimeline()
        unwindStack()
        favoritesTabBar.setCurrentIndex(0)
    }

    function viewPostFeed(feed, viewByDid = "") {
        const sw = getSkywalker(viewByDid)
        const modelId = sw.createPostFeedModel(feed)
        sw.getFeed(modelId)
        let component = guiSettings.createComponent("PostFeedView.qml")
        let view = component.createObject(root, { userDid: viewByDid, modelId: modelId })
        view.onClosed.connect(() => { popStack() })
        root.pushStack(view)
    }

    function viewMediaFeed(model, index, closeCb = (newIndex) => {}, viewByDid = "") {
        console.debug("View media feed:", model.feedName, "index:", index)
        let component = guiSettings.createComponent("MediaFeedView.qml")
        let view = component.createObject(root, { userDid: viewByDid, model: model, currentIndex: index })
        view.onClosed.connect(() => {
            console.debug("Close media feed:", model.feedName, "index:", view.currentIndex)
            closeCb(view.currentIndex)
            popStack(null, StackView.Immediate)
        })
        root.pushStack(view, StackView.Immediate)
    }

    function viewQuotePostFeed(quoteUri, viewByDid = "") {
        const sw = getSkywalker(viewByDid)
        const modelId = sw.createQuotePostFeedModel(quoteUri)
        sw.getQuotesFeed(modelId)
        let component = guiSettings.createComponent("QuotePostFeedView.qml")
        let view = component.createObject(root, { userDid: viewByDid, modelId: modelId })
        view.onClosed.connect(() => { popStack() }) // qmllint disable missing-property
        root.pushStack(view)
    }

    function viewListByUri(listUri, viewPosts, viewByDid = "") {
        const gu = getGraphUtils(viewByDid)
        gu.getListView(listUri, viewPosts)
    }

    function viewList(list, viewByDid = "") {
        switch (list.purpose) {
        case QEnums.LIST_PURPOSE_CURATE:
            viewPostListFeed(list, viewByDid)
            break
        case QEnums.LIST_PURPOSE_MOD:
            viewListFeedDescription(list, viewByDid)
            break
        }
    }

    function viewPostListFeed(list, viewByDid = "") {
        const sw = getSkywalker(viewByDid)
        const modelId = sw.createPostFeedModel(list)
        sw.getListFeed(modelId)
        let component = guiSettings.createComponent("PostFeedView.qml")
        let view = component.createObject(root, { userDid: viewByDid, modelId: modelId })
        view.onClosed.connect(() => { popStack() })
        root.pushStack(view)
    }

    function viewStarterPack(starterPack, viewByDid = "") {
        let component = guiSettings.createComponent("StarterPackView.qml")
        let view = component.createObject(root, { userDid: viewByDid, starterPack: starterPack })
        view.onClosed.connect(() => { popStack() }) // qmllint disable missing-property
        root.pushStack(view)
    }

    function viewAuthor(profile, modelId, viewByDid) {
        let component = guiSettings.createComponent("AuthorView.qml")
        let view = component.createObject(root, {
                userDid: viewByDid,
                author: profile,
                modelId: modelId,
        })
        view.onClosed.connect(() => { popStack() })
        pushStack(view)
    }

    function viewAuthorList(modelId, title, description = "", showFollow = true, showActivitySubscription = false, viewByDid = "") {
        let component = guiSettings.createComponent("AuthorListView.qml")
        let view = component.createObject(root, {
                title: title,
                modelId: modelId,
                userDid: viewByDid,
                description: description,
                showFollow: showFollow,
                showActivitySubscription: showActivitySubscription
        })
        view.onClosed.connect(() => { popStack() })
        pushStack(view)
        getSkywalker(viewByDid).getAuthorList(modelId)
    }

    function viewAuthorListByUser(viewByDid, modelId, title) {
        viewAuthorList(modelId, title, "", true, false, viewByDid)
    }

    function viewSimpleAuthorList(title, profiles, viewedByDid = "") {
        let component = guiSettings.createComponent("SimpleAuthorListPage.qml")
        let view = component.createObject(root, {
                title: title,
                userDid: viewedByDid,
                model: profiles
        })
        view.onClosed.connect(() => { popStack() }) // qmllint disable missing-property
        pushStack(view)
    }

    function viewUserLists(modelId) {
        let component = guiSettings.createComponent("UserListsPage.qml")
        let page = component.createObject(root, {
                modelId: modelId
        })
        page.onClosed.connect(() => { popStack() }) // qmllint disable missing-property
        pushStack(page)
        skywalker.getListList(modelId)
    }

    function viewModerationLists(modelId) {
        let component = guiSettings.createComponent("ModerationListsPage.qml")
        let page = component.createObject(root, {
                modelId: modelId
        })
        page.onClosed.connect(() => { popStack() }) // qmllint disable missing-property
        pushStack(page)
        skywalker.getListList(modelId)
    }

    function viewFeedDescription(feed, viewByDid = "") {
        let component = guiSettings.createComponent("FeedDescriptionView.qml")
        let view = component.createObject(root, { feed: feed, userDid: viewByDid })
        view.onClosed.connect(() => { popStack() }) // qmllint disable missing-property
        pushStack(view)
    }

    function viewListFeedDescription(list, viewByDid = "") {
        let component = guiSettings.createComponent("ListFeedDescriptionView.qml")
        let view = component.createObject(root, { list: list, skywalker: skywalker })
        view.onClosed.connect(() => { popStack() }) // qmllint disable missing-property
        pushStack(view)
    }

    function editSettings() {
        let component = guiSettings.createComponent("SettingsForm.qml")
        let form = component.createObject(root)
        form.onClosed.connect(() => { popStack() }) // qmllint disable missing-property
        pushStack(form)
    }

    function editAdvancedSettings() {
        let component = guiSettings.createComponent("AdvancedSettingsForm.qml")
        let form = component.createObject(root)
        form.onClosed.connect(() => { popStack() })
        pushStack(form)
    }

    function editChatSettings() {
        let component = guiSettings.createComponent("SettingsForm.qml")
        let form = component.createObject(root, { allVisible: false, onlyChatVisible: true })
        form.onClosed.connect(() => { popStack() }) // qmllint disable missing-property
        pushStack(form)
    }

    function editNotificationSettings() {
        let component = guiSettings.createComponent("SettingsForm.qml")
        let form = component.createObject(root, { allVisible: false, onlyNotificationVisible: true })
        form.onClosed.connect(() => { popStack() }) // qmllint disable missing-property
        pushStack(form)
    }

    function editContentFilterSettings() {
        let component = guiSettings.createComponent("ContentFilterSettings.qml")
        let contentGroupListModel = skywalker.getGlobalContentGroupListModel()
        let labelerModelId = skywalker.createAuthorListModel(QEnums.AUTHOR_LIST_LABELERS, "")
        let form = component.createObject(root, {
                globalLabelModel: contentGroupListModel,
                labelerAuthorListModelId: labelerModelId })
        form.onClosed.connect(() => { popStack() })
        pushStack(form)
        skywalker.getAuthorList(labelerModelId)
    }

    function reportAuthor(author, reportByDid = "") {
        let component = guiSettings.createComponent("Report.qml")
        let form = component.createObject(root, { userDid: reportByDid, author: author })
        form.onClosed.connect(() => { popStack() }) // qmllint disable missing-property
        pushStack(form)
    }

    function reportPost(postUri, postCid, postText, postDateTime, author, reportByDid = "") {
        let component = guiSettings.createComponent("Report.qml")
        let form = component.createObject(root, {
                userDid: reportByDid,
                postUri: postUri,
                postCid: postCid,
                postText: postText,
                postDateTime: postDateTime,
                author: author })
        form.onClosed.connect(() => { popStack() }) // qmllint disable missing-property
        pushStack(form)
    }

    function reportFeed(feed, reportByDid = "") {
        let component = guiSettings.createComponent("Report.qml")
        let form = component.createObject(root, { userDid: reportByDid, feed: feed })
        form.onClosed.connect(() => { popStack() }) // qmllint disable missing-property
        pushStack(form)
    }

    function reportList(list, reportByDid = "") {
        let component = guiSettings.createComponent("Report.qml")
        let form = component.createObject(root, { userDid: reportByDid, list: list })
        form.onClosed.connect(() => { popStack() }) // qmllint disable missing-property
        pushStack(form)
    }

    function reportStarterPack(starterPack, reportByDid = "") {
        let component = guiSettings.createComponent("Report.qml")
        let form = component.createObject(root, { userDid: reportByDid, starterPack: starterPack })
        form.onClosed.connect(() => { popStack() }) // qmllint disable missing-property
        pushStack(form)
    }

    function newList(listModel, purpose = QEnums.LIST_PURPOSE_UNKNOWN, userDid = "") {
        const sw = getSkywalker(userDid)
        let component = guiSettings.createComponent("EditList.qml")
        let page = component.createObject(root, {
                skywalker: sw,
                purpose: purpose !== QEnums.LIST_PURPOSE_UNKNOWN ? purpose : listModel.getPurpose()
            })
        page.onListCreated.connect((list) => {
            if (list.isNull()) {
                // This should rarely happen. Let the user refresh.
                sw.showStatusMessage(qsTr("List created. Please refresh page."), QEnums.STATUS_LEVEL_INFO);
            }
            else {
                sw.showStatusMessage(qsTr("List created."), QEnums.STATUS_LEVEL_INFO, 2)
                listModel.prependList(list)
            }

            root.popStack()
        })
        page.onClosed.connect(() => { root.popStack() })
        root.pushStack(page)
    }

    function translateText(text) {
        if (Qt.platform.os === "android") {
            if (utils.translate(text))
                return
        }

        const lang = Qt.locale().name.split("_")[0]
        const normalized = UnicodeFonts.normalizeToNFKD(text)
        const encoded = encodeURIComponent(normalized)
        const url = `https://translate.google.com/?hl=${lang}&sl=auto&tl=${lang}&text=${encoded}&op=translate`
        Qt.openUrlExternally(url)
    }

    function getFavoritesSwipeView() {
        return timelineStack.get(0)
    }

    function getTimelineView() {
        return getFavoritesSwipeView().itemAt(0)
    }

    function getNotificationView() {
        return notificationStack.get(0)
    }

    function getChatView() {
        return chatStack.get(0)
    }

    function currentStack() {
        return rootContent.children[rootContent.currentIndex]
    }

    function currentStackItem() {
        let stack = currentStack()
        return getStackTopItem(stack)
    }

    function getStackTopItem(stack) {
        if (stack.depth > 0)
            return stack.get(stack.depth - 1)

        return null
    }

    function currentStackIsChat() {
        return rootContent.currentIndex === rootContent.chatIndex
    }

    function popStack(stack = null, operation = StackView.PopTransition) {
        if (!stack)
            stack = currentStack()

        let item = stack.pop(operation)
        item.destroy()

        if (stack === currentStack()) {
            let currentItem = currentStackItem()

            if (currentItem && typeof currentItem.uncover === 'function')
                currentItem.uncover()
        }

        favoritesTabBar.update()
    }

    function pushStack(item, operation = StackView.PushTransition) {
        let current = currentStackItem()

        if (current && typeof current.cover === 'function')
            current.cover()

        currentStack().pushItem(item, {}, operation)
        favoritesTabBar.update()
    }

    function unwindStack(stack = null) {
        if (!stack)
            stack = currentStack()

        while (stack.depth > 1)
            popStack(stack)
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
    }

    Material.onThemeChanged: {
        let userSettings = skywalker.getUserSettings()
        const oldDisplayMode = userSettings.getActiveDisplayMode()
        const newDisplayMode = (root.Material.theme === Material.Light ? QEnums.DISPLAY_MODE_LIGHT : QEnums.DISPLAY_MODE_DARK)

        console.debug("Theme changed:", root.Material.theme, "old display mode:", oldDisplayMode, "new:", newDisplayMode)

        userSettings.setDefaultBackgroundColor(Material.background)
        userSettings.setActiveDisplayMode(newDisplayMode)
        userSettings.setCurrentLinkColor(guiSettings.linkColor)
        root.Material.accent = guiSettings.accentColor
        displayUtils.setNavigationBarColor(guiSettings.backgroundColor)
        displayUtils.setStatusBarColor(guiSettings.headerColor)

        // Refreshing the models makes them format text with the new colors (e.g. link color)
        if (oldDisplayMode !== newDisplayMode)
            skywalker.refreshAllModels()
    }

    function getSkywalker(did = "") {
        if (isActiveUser(did))
            return skywalker

        const sw = didSkywalkerMap.get(did)

        if (!sw)
            console.warn("No skywalker for:", did)

        return sw
    }

    function getPostUtils(did = "") {
        if (isActiveUser(did))
            return postUtils

        const pu = didPostUtilsMap.get(did)

        if (!pu)
            console.warn("No postUtils for:", did)

        return pu
    }

    function getProfileUtils(did = "") {
        if (isActiveUser(did))
            return profileUtils

        const profUtils = didProfileUtilsMap.get(did)

        if (!profUtils)
            console.warn("No profileUtils for:", did)

        return profUtils
    }

    function getGraphUtils(did = "") {
        if (isActiveUser(did))
            return graphUtils

        const gu = didGraphUtilsMap.get(did)

        if (!gu)
            console.warn("No graphUtils for:", did)

        return gu
    }

    function getFeedUtils(did = "") {
        if (isActiveUser(did))
            return feedUtils

        const fu = didFeedUtilsMap.get(did)

        if (!fu)
            console.warn("No feedUtils for:", did)

        return fu
    }

    function getLinkUtils(did = "") {
        if (isActiveUser(did))
            return linkUtils

        const lu = didLinkUtilsMap.get(did)

        if (!lu)
            console.warn("No linkUtils for:", did)

        return lu
    }

    function chatOnStartConvoForMembersOk(convo, msg) {
        let component = guiSettings.createComponent("MessagesListView.qml")
        let view = component.createObject(root, { chat: skywalker.chat, convo: convo })
        view.onClosed.connect((lastMessageId) => root.popStack())

        skywalker.chat.getMessages(convo.id)
        root.pushStack(view)

        if (msg)
            view.addMessage(msg)
    }

    function chatOnStartConvoForMembersFailed(error) {
        skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
    }

    function initHandlers() {
        skywalker.chat.onStartConvoForMembersOk.connect(chatOnStartConvoForMembersOk)
        skywalker.chat.onStartConvoForMembersFailed.connect(chatOnStartConvoForMembersFailed)
        skywalker.favoriteFeeds.onInitialized.connect(showLastViewedFeed)
    }

    Component.onCompleted: {
        console.debug("DPR:", Screen.devicePixelRatio)
        console.debug("Screen:", Screen.width, "x", Screen.height)
        console.debug("Font pt:", Qt.application.font.pointSize) // qmllint disable missing-property
        console.debug("Font px:", Qt.application.font.pixelSize)
        console.debug(Qt.fontFamilies());

        const userSettings = skywalker.getUserSettings()
        setDisplayMode(userSettings.getDisplayMode())

        let timelineComponent = guiSettings.createComponent("FavoritesSwipeView.qml")
        let timelinePage = timelineComponent.createObject(root, { skywalker: skywalker })
        timelinePage.onCurrentIndexChanged.connect(() => { favoritesTabBar.setCurrentIndex(timelinePage.currentIndex) })
        timelineStack.push(timelinePage)
        favoritesTabBar.favoritesSwipeView = timelinePage

        let notificationsComponent = guiSettings.createComponent("NotificationListView.qml")
        let notificationsView = notificationsComponent.createObject(root,
                { skywalker: skywalker, timeline: timelinePage })
        notificationsView.onClosed.connect(() => { rootContent.currentIndex = rootContent.timelineIndex })
        notificationStack.push(notificationsView)

        let chatComponent = guiSettings.createComponent("ConvoListView.qml")
        let chatView = chatComponent.createObject(root, { chat: skywalker.chat })
        chatView.onClosed.connect(() => { rootContent.currentIndex = rootContent.timelineIndex })
        chatStack.push(chatView)

        favoritesTabBar.update()

        initHandlers()

        // Try to resume the previous session. If that fails, then ask the user to login.
        if (skywalker.resumeAndRefreshSession())
            showStartupStatus()
        else if (skywalker.autoLogin())
            showStartupStatus()
        else
            signIn()

        // NOTE: the user is not yet logged in, but global app settings are available.
        let settings = root.getSkywalker().getUserSettings()
        postButtonRelativeX = settings.getPostButtonRelativeX()
    }
}
