import QtQuick
import QtQuick.Controls
import skywalker

SkyDialog {
    id: dialog
    x: 20
    y: guiSettings.headerHeight
    width: parent.width - 40
    title: qsTr("New group chat")
    standardButtons: Dialog.Ok | Dialog.Cancel

    SkyTextInput {
        id: textInput
        width: parent.width
        placeholderText: qsTr("Name of group")
        maximumLength: 128
    }

    function getText() {
        return textInput.text.trim()
    }

    Component.onCompleted: {
        textInput.setFocus()
    }
}
