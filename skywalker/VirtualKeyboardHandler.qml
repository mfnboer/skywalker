import QtQuick

Item {
    property bool keyboardVisible: Qt.inputMethod.visible && Qt.inputMethod.keyboardRectangle.y > 0 // qmllint disable missing-property

    // HACK: sometimes when the keyboard pops up, inputMethod visible becomes true
    // and then quickly false again. In that case the keyboard is visible on the
    // screen, but the keyboarRectangle is null so the screen does not move up.
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
