import QtQuick
import QtQuick.Controls
import skywalker

Rectangle {
    property var parentFlick
    property SvgImage svgIcon
    property string initialText
    property string placeholderText
    property alias echoMode: textField.echoMode
    property alias inputMethodHints: textField.inputMethodHints
    property alias maximumLength: textField.maximumLength
    property int graphemeLength: 0
    property int maximumGraphemeLength: -1
    property bool strictGraphemeMax: false
    property bool valid: true
    property alias padding: textField.padding
    property alias validator: textField.validator
    property alias text: textField.text
    property alias displayText: textField.displayText
    property alias textInput: textField

    signal editingFinished

    id: skyTextInput
    implicitHeight: textField.implicitHeight
    radius: 5
    border.width: textField.activeFocus ? 1 : 0
    border.color: guiSettings.buttonColor
    color: valid ? guiSettings.textInputBackgroundColor : guiSettings.textInputInvalidColor

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
            graphemeLength = UnicodeFonts.graphemeLength(text) +
                    UnicodeFonts.graphemeLength(preeditText)

            if (strictGraphemeMax && maximumGraphemeLength > -1 && graphemeLength > maximumGraphemeLength) {
                Qt.inputMethod.commit()
                const graphemeInfo = UnicodeFonts.getGraphemeInfo(text)
                text = graphemeInfo.sliced(text, 0, maximumGraphemeLength)
            }
        }

        onEditingFinished: skyTextInput.editingFinished()

        onCursorRectangleChanged: {
            ensureVisible(cursorRectangle)
        }

        onFocusChanged: {
            if (focus)
                ensureVisible(cursorRectangle)
        }

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
            svg: visible ? svgIcon : SvgOutline.check
            visible: svgIcon && !svgIcon.isNull()
            Accessible.ignored: true
        }


        AccessibleText {
            anchors.fill: parent
            padding: parent.padding
            leftPadding: parent.leftPadding
            font.pointSize: parent.font.pointSize
            color: guiSettings.placeholderTextColor
            text: placeholderText
            visible: parent.displayText.length === 0
            Accessible.ignored: true
        }

        function ensureVisible(cursor) {
            if (!parentFlick || parentFlick.dragging)
                return

            const editTextY = textField.mapToItem(parentFlick, 0, 0).y
            let cursorY = cursor.y + editTextY

            if (cursorY < 0)
                parentFlick.contentY += cursorY;
            else if (parentFlick.height < cursorY + cursor.height + textField.bottomPadding)
                parentFlick.contentY += cursorY + cursor.height + textField.bottomPadding - parentFlick.height
        }
    }

    function maxGraphemeLengthExceeded() {
        return maximumGraphemeLength > -1 && graphemeLength > maximumGraphemeLength
    }

    function setFocus() {
        textField.focus = true
    }

    function clear() {
        textField.clear()
    }

    Component.onCompleted: {
        if (parentFlick) {
            parentFlick.onHeightChanged.connect(() => {
                if (textField.activeFocus)
                    textField.ensureVisible(textField.cursorRectangle)
            })
        }
    }
}
