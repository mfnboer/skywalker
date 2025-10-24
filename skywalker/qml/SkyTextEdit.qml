import QtQuick
import skywalker

TextEdit {
    property string initialText
    property string placeholderText
    property double placeholderPointSize: guiSettings.scaledFont(9/8)
    property bool singleLine: false
    property int graphemeLength: 0
    property int maxLength: -1
    property bool strictMax: false
    property var fontSelectorCombo
    property var parentFlick
    property bool textChangeInProgress: false

    id: skyTextEdit
    width: page.width
    leftPadding: 10
    rightPadding: 10
    textFormat: TextEdit.PlainText
    wrapMode: TextEdit.Wrap
    font.pointSize: guiSettings.scaledFont(9/8)
    color: guiSettings.textColor
    selectionColor: guiSettings.selectionColor
    clip: true
    focus: true
    text: initialText

    Accessible.role: Accessible.EditableText
    Accessible.name: placeholder.visible ? placeholderText : text
    Accessible.multiLine: true
    Accessible.editable: true

    onTextChanged: {
        if (textChangeInProgress)
            return

        const added = updateGraphemeLength()

        if (added > 0)
            updateTextTimer.set(added)

        if (!singleLine)
            return

        const index = text.indexOf("\n")

        if (index >= 0)
            remove(index, text.length)
    }

    onPreeditTextChanged: {
        if (textChangeInProgress)
            return

        const added = updateGraphemeLength()

        if (added > 0)
            updateTextTimer.set(added)
    }

    onCursorRectangleChanged: {
        ensureVisible(cursorRectangle)
    }

    onFocusChanged: {
        if (focus)
            ensureVisible(cursorRectangle)
    }

    function updateGraphemeLength() {
        const prevGraphemeLength = graphemeLength
        graphemeLength = UnicodeFonts.graphemeLength(skyTextEdit.text) +
                UnicodeFonts.graphemeLength(preeditText)

        if (strictMax && maxLength > -1 && graphemeLength > maxLength) {
            Qt.inputMethod.commit()
            const graphemeInfo = UnicodeFonts.getGraphemeInfo(text)
            text = graphemeInfo.sliced(text, 0, maxLength)
            cursorPosition = text.length
            graphemeLength = maxLength
        }

        return graphemeLength - prevGraphemeLength
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
        color: guiSettings.placeholderTextColor
        elide: Text.ElideRight
        text: placeholderText
        visible: skyTextEdit.length + skyTextEdit.preeditText.length === 0
    }

    function maxGraphemeLengthExceeded() {
        return maxLength > -1 && graphemeLength > maxLength
    }

    EmojiFixHighlighter {
        id: emojiFixer
    }

    FacetUtils {
        id: facetUtils
        skywalker: root.getSkywalker()
    }

    // Text can only be changed outside onPreeditTextChanged.
    // This timer makes the call to applyFont async.
    Timer {
        property int numChars: 1

        id: updateTextTimer
        interval: 0
        onTriggered: {
            skyTextEdit.textChangeInProgress = true
            skyTextEdit.applyFont(numChars)
            skyTextEdit.textChangeInProgress = false
        }

        function set(num) {
            if (!fontSelectorCombo || fontSelectorCombo.currentIndex === QEnums.FONT_NORMAL)
                return

            numChars = num
            start()
        }
    }

    function applyFont(numChars) {
        if (!fontSelectorCombo)
            return

        const modifiedTillCursor = facetUtils.applyFontToLastTypedChars(
                                     skyTextEdit.text, skyTextEdit.preeditText,
                                     skyTextEdit.cursorPosition, numChars,
                                     fontSelectorCombo.currentIndex)

        if (modifiedTillCursor) {
            const fullText = modifiedTillCursor + skyTextEdit.text.slice(skyTextEdit.cursorPosition)
            skyTextEdit.clear()
            skyTextEdit.text = fullText
            skyTextEdit.cursorPosition = modifiedTillCursor.length
        }
    }

    function ensureVisible(cursor) {
        if (!parentFlick || parentFlick.dragging)
            return

        const editTextY = skyTextEdit.mapToItem(parentFlick, 0, 0).y
        let cursorY = cursor.y + editTextY

        if (cursorY < 0)
            parentFlick.contentY += cursorY;
        else if (parentFlick.height < cursorY + cursor.height + skyTextEdit.bottomPadding)
            parentFlick.contentY += cursorY + cursor.height + skyTextEdit.bottomPadding - parentFlick.height
    }

    Component.onCompleted: {
        emojiFixer.setEmojiFixDocument(textDocument, maxLength, guiSettings.textLengthExceededColor)
        cursorPosition = text.length
    }
}
