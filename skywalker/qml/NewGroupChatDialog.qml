import QtQuick
import QtQuick.Controls
import skywalker

SkyDialog {
    property convoview convo
    property Skywalker skywalker: root.getSkywalker()

    id: dialog
    x: 20
    y: guiSettings.headerHeight
    width: parent.width - 40
    title: convo.isNull() ? qsTr("Create group chat") : qsTr("Edit group chat")
    standardButtons: Dialog.Ok | Dialog.Cancel

    SkyTextInput {
        id: textInput
        width: parent.width
        placeholderText: qsTr("Name of group")
        initialText: convo.group.name
        maximumLength: convo.isNull() ? skywalker.chat.MAX_CREATE_GROUP_NAME_LEN : skywalker.chat.MAX_EDIT_GROUP_NAME_LEN
    }

    function getText() {
        return textInput.text.trim()
    }

    Component.onCompleted: {
        textInput.setFocus()
    }
}
