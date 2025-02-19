import QtQuick
import skywalker

Item {
    property int rootHeight
    property bool keyboardVisible: receivedKeyboardHeight > 0 || Qt.inputMethod.visible
    property int receivedKeyboardHeight: 0
    property int keyboardHeight: 0

    onReceivedKeyboardHeightChanged: {
        if (receivedKeyboardHeight > 0) {
            updateTimer.start()
        }
        else {
            keyboardHeight = 0;
            updateTimer.stop()
        }
    }

    onKeyboardHeightChanged: root.height = rootHeight - keyboardHeight

    // Qt.inputMethod is not reliable
    VirtualKeyboardUtils {
        onKeyboardHeightChanged: (height) => receivedKeyboardHeight = Math.ceil(height / Screen.devicePixelRatio)
    }

    // Throttle updates to avoid screen flicker
    Timer {
        id: updateTimer
        interval: 300
        onTriggered: keyboardHeight = receivedKeyboardHeight
    }

    Component.onDestruction: {
        root.height = rootHeight
    }

    Component.onCompleted: {
        rootHeight = root.height
    }
}
