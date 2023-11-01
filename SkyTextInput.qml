import QtQuick
import QtQuick.Controls
import skywalker

Rectangle {
    property svgimage svgIcon
    property string initialText
    property string placeholderText
    property int echoMode
    property alias text: textField.text

    id: skyTextInput
    height: textField.height
    radius: 10
    border.width: 1
    border.color: "grey"

    TextInput {
        id: textField
        width: parent.width
        padding: 10
        leftPadding: icon.width
        activeFocusOnTab: true
        echoMode: skyTextInput.echoMode
        inputMethodHints: Qt.ImhNoAutoUppercase
        enabled: skyTextInput.enabled
        font.pointSize: guiSettings.scaledFont(9/8)
        text: skyTextInput.initialText

        Text {
            anchors.fill: parent
            padding: parent.padding
            leftPadding: parent.leftPadding
            font.pointSize: parent.font.pointSize
            color: "grey"
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
        }
    }

    GuiSettings {
        id: guiSettings
    }

    function setFocus() {
        textField.focus = true
    }
}
