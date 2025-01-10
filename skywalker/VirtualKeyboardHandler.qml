import QtQuick

Item {
    property int pageHeight: root.height
    property bool keyboardVisible: Qt.inputMethod.visible && Qt.inputMethod.keyboardRectangle.y > 0 // qmllint disable missing-property
    property int keyboardY: Qt.inputMethod.keyboardRectangle.y / Screen.devicePixelRatio // qmllint disable missing-property
    property int keyboardHeight: keyboardVisible ? pageHeight - keyboardY : 0

    // HACK: sometimes when the keyboard pops up, inputMethod visible becomes true
    // and then quickly false again. In that case the keyboard is visible on the
    // screen, but the keyboarRectangle in null so the screen does no move up.
    // To avoid this we force hide the keyboard. The user has to tap once more.
    Connections {
        target: Qt.inputMethod

        function onVisibleChanged() {
            if (!Qt.inputMethod.visible) { // qmllint disable missing-property
                console.debug("KEYBOARD FORCE HIDE")
                Qt.inputMethod.hide()
            }
        }
    }
}
