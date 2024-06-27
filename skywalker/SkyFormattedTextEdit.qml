import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

TextEdit {
    required property var parentPage
    required property var parentFlick
    property string initialText: ""
    property string placeholderText
    property int graphemeLength: 0
    property int maxLength: -1
    property bool enableLinkShortening: true
    property var fontSelectorCombo
    property bool textChangeInProgress: false
    property string firstWebLink
    property bool cursorInFirstWebLink: false
    property string firstPostLink
    property bool cursorInFirstPostLink: false
    property string firstFeedLink
    property bool cursorInFirstFeedLink: false
    property string firstListLink
    property bool cursorInFirstListLink: false
    property int prevTextLen: 0
    property int lastDeltaTextLen: 0

    id: editText
    width: parentPage.width
    height: graphemeLength === 0 ? placeholderTextField.height : implicitHeight
    leftPadding: 10
    rightPadding: 10
    activeFocusOnTab: true
    textFormat: TextEdit.PlainText
    wrapMode: TextEdit.Wrap
    font.pointSize: guiSettings.scaledFont(9/8)
    color: guiSettings.textColor
    selectionColor: guiSettings.selectionColor
    clip: true
    text: initialText

    Accessible.role: Accessible.EditableText
    Accessible.name: text ? text : qsTr(`Enter ${placeholderText}`)
    Accessible.description: Accessible.name
    Accessible.editable: true
    Accessible.multiLine: true

    onCursorRectangleChanged: {
        let editMentionY = postUtils.editMentionCursorY
        let editTagY = postUtils.editTagCursorY
        let cursorY = cursorRectangle.y

        if (postUtils.editMention.length > 0 && editMentionY != cursorY)
            postUtils.editMention = ""

        if (postUtils.editTag.length > 0 && editTagY != cursorY)
            postUtils.editTag = ""

        ensureVisible(cursorRectangle)
    }

    onFocusChanged: {
        if (focus)
            ensureVisible(cursorRectangle)
    }

    onTextChanged: {
        if (textChangeInProgress)
            return

        textChangeInProgress = true

        if (text.length !== prevTextLen) {
            lastDeltaTextLen = text.length - prevTextLen
            prevTextLen = text.length
        }

        highlightFacets()

        const added = updateGraphemeLength()
        if (added > 0)
            updateTextTimer.set(added)

        textChangeInProgress = false
    }

    onPreeditTextChanged: {
        if (textChangeInProgress)
            return

        const added = updateGraphemeLength()
        if (added > 0)
            updateTextTimer.set(added)
    }

    onMaxLengthChanged: {
        postUtils.setHighlightDocument(editText.textDocument, guiSettings.linkColor,
                                       editText.maxLength, guiSettings.textLengthExceededColor)
    }

    Keys.onReleased: (event) => {
        // Work around for Qt6.7.2 which hides the keyboard on pressing return.
        if (event.key === Qt.Key_Return)
            Qt.inputMethod.show()
    }

    // Text can only be changed outside onPreeditTextChanged.
    // This timer makes the call to applyFont async.
    Timer {
        property int numChars: 1

        id: updateTextTimer
        interval: 0
        onTriggered: {
            editText.textChangeInProgress = true
            editText.applyFont(numChars)
            editText.textChangeInProgress = false
        }

        function set(num) {
            numChars = num
            start()
        }
    }

    function applyFont(numChars) {
        if (!fontSelectorCombo)
            return

        const modifiedTillCursor = postUtils.applyFontToLastTypedChars(
                                     editText.text, editText.preeditText,
                                     editText.cursorPosition, numChars,
                                     fontSelectorCombo.currentIndex)

        if (modifiedTillCursor) {
            const fullText = modifiedTillCursor + editText.text.slice(editText.cursorPosition)
            editText.clear()
            editText.text = fullText
            editText.cursorPosition = modifiedTillCursor.length
        }
    }

    function highlightFacets() {
        postUtils.extractMentionsAndLinks(editText.text,
                editText.preeditText, cursorPosition)
    }

    function updateGraphemeLength() {
        const prevGraphemeLength = graphemeLength
        const linkShorteningReduction = enableLinkShortening ? postUtils.getLinkShorteningReduction() : 0

        graphemeLength = unicodeFonts.graphemeLength(editText.text) +
                unicodeFonts.graphemeLength(preeditText) -
                linkShorteningReduction

        postUtils.setHighLightMaxLength(editText.maxLength + linkShorteningReduction)

        return graphemeLength - prevGraphemeLength
    }

    function ensureVisible(cursor) {
        if (parentFlick.dragging)
            return

        const editTextY = editText.mapToItem(parentFlick, 0, 0).y
        let cursorY = cursor.y + editTextY

        if (cursorY < 0)
            parentFlick.contentY += cursorY;
        else if (parentFlick.height < cursorY + cursor.height)
            parentFlick.contentY += cursorY + cursor.height - parentFlick.height
    }

    function isCursorVisible() {
        const editTextY = editText.mapToItem(parentFlick, 0, 0).y
        let cursorY = cursorRectangle.y + editTextY
        return cursorY >= 0 && parentFlick.height >= cursorY + cursorRectangle.height
    }

    // Set cursor in the center position if it has become invisible
    function resetCursorPosition() {
        if (!isCursorVisible()) {
            const point = editText.mapFromItem(parentFlick, parentFlick.width / 2, parentFlick.height / 2)
            cursorPosition = positionAt(point.x, point.y)
        }
    }

    // Cut a link if it was just completely added (paste or share) and sits right
    // before the cursor.
    // Returns true if the cut was made.
    function cutLinkIfJustAdded(link, cbBeforeCut) {
        if (lastDeltaTextLen < link.length)
            return false

        if (cursorPosition < link.length)
            return false

        Qt.inputMethod.commit()
        const startIndex = cursorPosition - link.length
        const lastAdded = text.substring(startIndex, cursorPosition)

        if (lastAdded !== link)
            return false

        cbBeforeCut()
        const newText = text.slice(0, startIndex) + text.slice(cursorPosition)
        clear()
        text = newText
        cursorPosition = startIndex

        return true
    }

    Text {
        id: placeholderTextField
        width: parent.width
        leftPadding: editText.leftPadding
        rightPadding: editText.rightPadding
        topPadding: editText.topPadding
        bottomPadding: editText.bottomPadding
        font.pointSize: editText.font.pointSize
        wrapMode: Text.Wrap
        color: guiSettings.placeholderTextColor
        text: placeholderText
        visible: editText.graphemeLength === 0
    }

    Timer {
        id: authorTypeaheadSearchTimer
        interval: 500
        onTriggered: {
            if (postUtils.editMention.length > 0)
                searchUtils.searchAuthorsTypeahead(postUtils.editMention, 10)
        }
    }

    Timer {
        id: hashtagTypeaheadSearchTimer
        interval: 500
        onTriggered: {
            if (postUtils.editTag.length > 0)
                searchUtils.searchHashtagsTypeahead(postUtils.editTag, 10)
        }
    }

    SearchUtils {
        id: searchUtils
        skywalker: parentPage.skywalker

        Component.onDestruction: {
            // The destuctor of SearchUtils is called too late by the QML engine
            // Remove models now before the Skywalker object is destroyed.
            searchUtils.removeModels()
        }
    }

    PostUtils {
        property double editMentionCursorY: 0
        property double editTagCursorY: 0

        id: postUtils
        skywalker: parentPage.skywalker

        onEditMentionChanged: {
            console.debug(editMention)
            editMentionCursorY = editText.cursorRectangle.y
            authorTypeaheadSearchTimer.start()
        }

        onEditTagChanged: {
            console.debug(editTag)
            editTagCursorY = editText.cursorRectangle.y
            hashtagTypeaheadSearchTimer.start()
        }

        onFirstWebLinkChanged: editText.firstWebLink = firstWebLink
        onCursorInFirstWebLinkChanged: editText.cursorInFirstWebLink = cursorInFirstWebLink
        onFirstPostLinkChanged: editText.firstPostLink = firstPostLink
        onCursorInFirstPostLinkChanged: editText.cursorInFirstPostLink = cursorInFirstPostLink
        onFirstFeedLinkChanged: editText.firstFeedLink = firstFeedLink
        onCursorInFirstFeedLinkChanged: editText.cursorInFirstFeedLink = cursorInFirstFeedLink
        onFirstListLinkChanged: editText.firstListLink = firstListLink
        onCursorInFirstListLinkChanged: editText.cursorInFirstListLink = cursorInFirstListLink
    }

    UnicodeFonts {
        id: unicodeFonts
    }

    GuiSettings {
        id: guiSettings
    }

    function maxGraphemeLengthExceeded() {
        return maxLength > -1 && graphemeLength > maxLength
    }

    function getTextParts() {
        const textBefore = editText.text.slice(0, editText.cursorPosition)
        const textBetween = editText.preeditText
        const textAfter = editText.text.slice(editText.cursorPosition)
        const fullText = textBefore + textBetween + textAfter

        return {textBefore, textBetween, textAfter, fullText}
    }

    function createAuthorTypeaheadView() {
        let component = Qt.createComponent("AuthorTypeaheadView.qml")
        let page = component.createObject(parentPage, {
                parentPage: parentPage,
                editText: editText,
                searchUtils: searchUtils,
                postUtils: postUtils
            })
    }

    function createHashtagTypeaheadView() {
        let component = Qt.createComponent("HashtagTypeaheadView.qml")
        let page = component.createObject(parentPage, {
                parentPage: parentPage,
                editText: editText,
                searchUtils: searchUtils,
                postUtils: postUtils
            })
    }

    Component.onCompleted: {
        createAuthorTypeaheadView()
        createHashtagTypeaheadView()
        postUtils.setHighlightDocument(editText.textDocument, guiSettings.linkColor,
                                       editText.maxLength, guiSettings.textLengthExceededColor)
    }
}

