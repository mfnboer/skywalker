import QtCore
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import skywalker

ApplicationWindow {
    property int headerHeight: 44
    property int footerHeight: 44

    id: root
    width: 480
    height: 960
    visible: true
    title: "Skywalker"

    onClosing: (event) => {
        // This catches the back-button on Android
        if (stack.depth > 1) {
            event.accepted = false
            popStack()
        }
    }

    StackView {
        id: stack
        initialItem: timelineView
        anchors.fill: parent
    }

    Component {
        id: timelineView

        TimelineView {}
    }

    SvgOutline {
        id: svgOutline
    }
    SvgFilled {
        id: svgFilled
    }

    StatusPopup {
        id: statusPopup
        z: 100
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

        onStatusMessage: (msg, level) => statusPopup.show(msg, level)
        onPostThreadOk: (modelId, postEntryIndex) => viewPostThread(modelId, postEntryIndex)
        onGetUserProfileOK: () => skywalker.syncTimeline()

        onGetUserProfileFailed: {
            // TODO: retry
            console.warn("FAILED TO LOAD USER PROFILE")
        }

        onTimelineSyncOK: (index) => {
            getTimelineView().setInSync(index)
            timelineUpdateTimer.start()
        }

        onTimelineSyncFailed: {
            console.warn("SYNC FAILED")
            getTimelineView().setInSync(0)
        }

        function start() {
            skywalker.getUserProfileAndFollows()
        }
    }

    Timer {
        id: timelineUpdateTimer
        // There is a trade off: short timeout is fast updating timeline, long timeout
        // allows for better reply thread construction as we receive more posts per update.
        interval: 91000
        running: false
        repeat: true
        onTriggered: skywalker.getTimelinePrepend(2)
    }

    function composePost() {
        let component = Qt.createComponent("ComposePost.qml")
        let page = component.createObject(root, {
                skywalker: skywalker,
        })
        page.onClosed.connect(() => { popStack() })
        stack.push(page)
    }

    function composeReply(replyToUri, replyToText, replyToDateTime, replyToAuthor) {
        let component = Qt.createComponent("ComposePost.qml")
        let page = component.createObject(root, {
                skywalker: skywalker,
                replyToPostUri: replyToUri,
                replyToPostText: replyToText,
                replyToPostDateTime: replyToDateTime,
                replyToAuthor: replyToAuthor
        })
        page.onClosed.connect(() => { popStack() })
        stack.push(page)
    }

    function viewPostThread(modelId, postEntryIndex) {
        let component = Qt.createComponent("PostThreadView.qml")
        let view = component.createObject(root, { modelId: modelId, postEntryIndex: postEntryIndex })
        view.onClosed.connect(() => { popStack() })
        stack.push(view)
    }

    function viewFullImage(imageList, currentIndex) {
        let component = Qt.createComponent("FullImageView.qml")
        let view = component.createObject(root, { images: imageList, imageIndex: currentIndex })
        view.onClosed.connect(() => { popStack() })
        stack.push(view)
    }

    function getTimelineView() {
        return stack.get(0)
    }

    function popStack() {
        let item = stack.pop()
        item.destroy()
    }

    function scaledFont(scaleFactor) {
        return Application.font.pointSize * scaleFactor;
    }

    Component.onCompleted: {
        // Try to resume the previous session. If that fails, then ask the user to login.
        skywalker.resumeSession()
    }
}
