import QtQuick
import skywalker

TextEdit {
    property string initialText
    property string placeholderText
    property double placeholderPointSize: GuiSettings.scaledFont(9/8)
    property bool singleLine: false
    property int graphemeLength: 0
    property int maxLength: -1
    property bool strictMax: false

    id: skyTextEdit
    width: page.width
    leftPadding: 10
    rightPadding: 10
    textFormat: TextEdit.PlainText
    wrapMode: TextEdit.Wrap
    font.pointSize: GuiSettings.scaledFont(9/8)
    color: GuiSettings.textColor
    selectionColor: GuiSettings.selectionColor
    clip: true
    focus: true
    text: initialText

    Accessible.role: Accessible.EditableText
    Accessible.name: placeholder.visible ? placeholderText : text
    Accessible.multiLine: true
    Accessible.editable: true

    onTextChanged: {
        updateGraphemeLength()

        if (!singleLine)
            return

        const index = text.indexOf("\n")

        if (index >= 0)
            remove(index, text.length)
    }

    onPreeditTextChanged: updateGraphemeLength()

    function updateGraphemeLength() {
        graphemeLength = unicodeFonts.graphemeLength(skyTextEdit.text) +
                unicodeFonts.graphemeLength(preeditText)

        if (strictMax && maxLength > -1 && graphemeLength > maxLength) {
            Qt.inputMethod.commit()
            const graphemeInfo = unicodeFonts.getGraphemeInfo(text)
            text = graphemeInfo.sliced(text, 0, maxLength)
            cursorPosition = text.length
        }
    }

    Text {
        id: placeholder
        anchors.fill: parent
        padding: parent.padding
        leftPadding: parent.leftPadding
        rightPadding: parent.rightPadding
        topPadding: parent.topPadding
        bottomPadding: parent.bottomPadding
        font.pointSize: placeholderPointSize
        color: GuiSettings.placeholderTextColor
        elide: Text.ElideRight
        text: placeholderText
        visible: skyTextEdit.length + skyTextEdit.preeditText.length === 0
    }

    function maxGraphemeLengthExceeded() {
        return maxLength > -1 && graphemeLength > maxLength
    }

    UnicodeFonts {
        id: unicodeFonts
    }


    Component.onCompleted: {
        unicodeFonts.setEmojiFixDocument(textDocument, maxLength, GuiSettings.textLengthExceededColor)
        cursorPosition = text.length
    }
}
