import QtQuick
import skywalker

Item {
    property bool keyboardVisible: keyboardHeight > 0
    property int keyboardHeight: 0

    // Qt.inputMethod is not reliable
    VirtualKeyboardUtils {
        onKeyboardHeightChanged: (height) => {
            keyboardHeight = Math.ceil(height / Screen.devicePixelRatio)
        }
    }
}
