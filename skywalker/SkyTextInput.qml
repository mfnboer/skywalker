import QtQuick
import QtQuick.Controls
import skywalker

Rectangle {
    property svgimage svgIcon
    property string initialText
    property string placeholderText
    property alias echoMode: textField.echoMode
    property alias inputMethodHints: textField.inputMethodHints
    property alias maximumLength: textField.maximumLength
    property alias validator: textField.validator
    property alias text: textField.text
    property alias displayText: textField.displayText

    id: skyTextInput
    height: textField.height
    radius: 10
    border.width: 1
    border.color: guiSettings.borderColor
    color: guiSettings.backgroundColor

    Accessible.role: Accessible.Pane

    TextInput {
        id: textField
        width: parent.width
        clip: true
        padding: 10
        leftPadding: icon.visible ? icon.width : 10
        activeFocusOnTab: true
        enabled: skyTextInput.enabled
        font.pointSize: guiSettings.scaledFont(9/8)
        color: guiSettings.textColor
        selectionColor: guiSettings.selectionColor
        text: skyTextInput.initialText

        Accessible.role: Accessible.EditableText
        Accessible.name: placeholderText
        Accessible.passwordEdit: echoMode === TextInput.Password
        Accessible.editable: enabled

        Text {
            anchors.fill: parent
            padding: parent.padding
            leftPadding: parent.leftPadding
            font.pointSize: parent.font.pointSize
            color: guiSettings.placeholderTextColor
            text: placeholderText
            visible: parent.displayText.length === 0
        }

        SvgImage {
            id: icon
            anchors.left: parent.left
            y: height + 5
            width: height
            height: parent.height - 10
            color: parent.color
            svg: svgIcon
            visible: !svgIcon.isNull()
        }
    }

    GuiSettings {
        id: guiSettings
    }

    function setFocus() {
        textField.focus = true
    }
}
