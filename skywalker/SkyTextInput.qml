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
    property int graphemeLength: 0
    property int maximumGraphemeLength: -1
    property bool strictGraphemeMax: false
    property alias validator: textField.validator
    property alias text: textField.text
    property alias displayText: textField.displayText

    signal editingFinished

    id: skyTextInput
    height: textField.height
    radius: 5
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
        color: enabled ? guiSettings.textColor : guiSettings.disabledColor
        selectionColor: guiSettings.selectionColor
        text: skyTextInput.initialText

        Accessible.role: Accessible.EditableText
        Accessible.name: text ? text : qsTr(`Enter ${placeholderText}`)
        Accessible.description: Accessible.name
        Accessible.editable: true
        Accessible.passwordEdit: echoMode === TextInput.Password

        onTextChanged: updateGraphemeLength()
        onPreeditTextChanged: updateGraphemeLength()

        function updateGraphemeLength() {
            graphemeLength = unicodeFonts.graphemeLength(text) +
                    unicodeFonts.graphemeLength(preeditText)

            if (strictGraphemeMax && maximumGraphemeLength > -1 && graphemeLength > maximumGraphemeLength) {
                Qt.inputMethod.commit()
                const graphemeInfo = unicodeFonts.getGraphemeInfo(text)
                text = graphemeInfo.sliced(text, 0, maximumGraphemeLength)
            }
        }

        onEditingFinished: skyTextInput.editingFinished()

        // Cover long text that may scroll underneath the icon
        Rectangle {
            radius: skyTextInput.radius
            x: 1
            y: 2
            width: icon.width - 1
            height: parent.height - 4
            color: skyTextInput.color
        }

        SkySvg {
            id: icon
            anchors.left: parent.left
            y: height + 5
            width: visible ? height : 0
            height: parent.height - 10
            color: parent.color
            svg: svgIcon
            visible: !svgIcon.isNull()
            Accessible.ignored: true
        }


        Text {
            anchors.fill: parent
            padding: parent.padding
            leftPadding: parent.leftPadding
            font.pointSize: parent.font.pointSize
            color: guiSettings.placeholderTextColor
            text: placeholderText
            visible: parent.displayText.length === 0
            Accessible.ignored: true
        }
    }

    function maxGraphemeLengthExceeded() {
        return maximumGraphemeLength > -1 && graphemeLength > maximumGraphemeLength
    }

    UnicodeFonts {
        id: unicodeFonts
    }

    GuiSettings {
        id: guiSettings
    }

    function setFocus() {
        textField.focus = true
    }

    function clear() {
        textField.clear()
    }
}
