import QtQuick

// Timer for moving to an index in a list or grid. Positioning often needs
// more than a single attempt as the list or grid gets shifted while entries
// are being rendered.
Timer {
    property int listIndex
    property int moveAttempt
    property var callback
    property var afterMoveCallback: () =>{}

    id: moveToIndexTimer
    interval: 200
    onTriggered: {
        if (!callback(listIndex)) { // qmllint disable use-proper-function
            if (moveAttempt < 5) {
                moveAttempt = moveAttempt + 1
                start()
            }
            else {
                console.debug("No exact move, no more attempts")
                afterMoveCallback()
                afterMoveCallback = () =>{}
            }
        }
        else {
            afterMoveCallback()
            afterMoveCallback = () =>{}
        }
    }

    function go(index, callbackFunc, afterMoveCb = () =>{}) {
        if (running) {
            console.debug("New move to:", index, "previous still running:", listIndex)
            afterMoveCallback()
            stop()
        }

        if (!callbackFunc(index)) {
            // HACK: doing it again after a short interval makes the positioning work.
            // After the first time the positioning can be off.
            listIndex = index
            moveAttempt = 2
            callback = callbackFunc
            afterMoveCallback = afterMoveCb
            start()
        }
        else {
            afterMoveCb()
        }
    }
}
