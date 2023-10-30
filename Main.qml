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
            event.accepted = false

            let item = currentStackItem()
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

    Login {
        id: loginDialog
        anchors.centerIn: parent
        onAccepted: skywalker.login(user, password, host)
    }

    Skywalker {
        id: skywalker
        onLoginOk: start()
        onLoginFailed: (error) => loginDialog.show(error)
        onResumeSessionOk: start()
        onResumeSessionFailed: loginDialog.show()

        onSessionExpired: (error) => {
            timelineUpdateTimer.stop()
            loginDialog.show()
        }

        onStatusMessage: (msg, level) => statusPopup.show(msg, level, level === QEnums.STATUS_LEVEL_INFO ? 2 : 30)
        onPostThreadOk: (modelId, postEntryIndex) => viewPostThread(modelId, postEntryIndex)
        onGetUserProfileOK: () => skywalker.getUserPreferences()

        onGetUserProfileFailed: {
            // TODO: retry
            console.warn("FAILED TO LOAD USER PROFILE")
            statusPopup.show("FAILED TO LOAD USER PROFILE", QEnums.STATUS_LEVEL_ERROR)
        }

        onGetUserPreferencesOK: () => skywalker.syncTimeline()

        onGetUserPreferencesFailed: {
            // TODO: retry
            console.warn("FAILED TO LOAD USER PREFERENCES")
            statusPopup.show("FAILED TO LOAD USER PREFERENCES", QEnums.STATUS_LEVEL_ERROR)
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

        onModeration: {
            editContentFilterSettings()
            close()
        }

        onSettings: {
            editSettings()
            close()
        }

        function show() {
            user = skywalker.getUser()
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

    GuiSettings {
        id: guiSettings
    }

    function showSettingsDrawer() {
        settingsDrawer.show()
    }

    function openLink(link) {
        linkUtils.openLink(link)
    }

    function composePost(initialText) {
        let component = Qt.createComponent("ComposePost.qml")
        let page = component.createObject(root, {
                skywalker: skywalker,
                initialText: initialText
        })
        page.onClosed.connect(() => { popStack() })
        currentStack().push(page)
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
        currentStack().push(page)
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
        currentStack().push(page)
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
        currentStack().push(view)
    }

    function viewFullImage(imageList, currentIndex) {
        let component = Qt.createComponent("FullImageView.qml")
        let view = component.createObject(root, { images: imageList, imageIndex: currentIndex })
        view.onClosed.connect(() => { popStack() })
        currentStack().push(view)
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
        currentStack().push(view)
    }

    function viewAuthorList(modelId, title) {
        let component = Qt.createComponent("AuthorListView.qml")
        let view = component.createObject(root, { title: title, modelId: modelId, skywalker: skywalker })
        view.onClosed.connect(() => { popStack() })
        currentStack().push(view)
        skywalker.getAuthorList(modelId, 50)
    }

    function editSettings() {
        let component = Qt.createComponent("SettingsForm.qml")
        let userPrefs = skywalker.getEditUserPreferences()
        let form = component.createObject(root, { userPrefs: userPrefs })
        form.onClosed.connect(() => { popStack() })
        currentStack().push(form)
    }

    function editContentFilterSettings() {
        let component = Qt.createComponent("ContentFilterSettings.qml")
        let contentGroupListModel = skywalker.getContentGroupListModel()
        let form = component.createObject(root, { model: contentGroupListModel })
        form.onClosed.connect(() => { popStack() })
        currentStack().push(form)
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
