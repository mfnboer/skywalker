import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Dialog {
    property string editWord

    width: parent.width
    contentHeight: textInput.height
    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel
    anchors.centerIn: parent

    Accessible.role: Accessible.Dialog

    SkyTextInput {
        id: textInput
        width: parent.width
        svgIcon: svgOutline.mutedWords
        initialText: editWord
        placeholderText: qsTr("Word/phrase to mute")
        enabled: true
    }

    function getText() {
        return textInput.text.trim()
    }

    function show() {
        open()
    }

    Component.onCompleted: {
        textInput.setFocus()
    }
}
