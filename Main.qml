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
        // This catches the back-button on Android
        if (stack.depth > 1) {
            event.accepted = false
            popStack()
        }
    }

    StackView {
        id: stack
        anchors.fill: parent
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
        onTriggered: {
            skywalker.getTimelinePrepend(2)
            skywalker.updatePostIndexTimestamps()
        }
    }

    function composePost() {
        let component = Qt.createComponent("ComposePost.qml")
        let page = component.createObject(root, {
                skywalker: skywalker,
        })
        page.onClosed.connect(() => { popStack() })
        stack.push(page)
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
        stack.push(page)
    }

    function composeQuote(quoteUri, quoteCid, quoteText, quoteDateTime, quoteAuthor) {
        let component = Qt.createComponent("ComposePost.qml")
        let page = component.createObject(root, {
                skywalker: skywalker,
                quoteUri: quoteUri,
                quoteCid: quoteCid,
                quoteText: quoteText,
                quoteDateTime: quoteDateTime,
                quoteAuthor: quoteAuthor
        })
        page.onClosed.connect(() => { popStack() })
        stack.push(page)
    }

    function repost(repostUri, uri, cid, text, dateTime, author) {
        let timeline = getTimelineView()
        timeline.repost(repostUri, uri, cid, text, dateTime, author)
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

    Component.onCompleted: {
        let component = Qt.createComponent("TimelineView.qml")
        let view = component.createObject(root, { skywalker: skywalker })
        stack.push(view)

        // Try to resume the previous session. If that fails, then ask the user to login.
        skywalker.resumeSession()
    }
}
