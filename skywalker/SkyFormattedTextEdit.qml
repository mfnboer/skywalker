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
    property var parentPostUtils
    property bool textChangeInProgress: false

    id: editText
    width: parentPage.width
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
        let editMentionY = getPostUtils().editMentionCursorY
        let editTagY = getPostUtils().editTagCursorY
        let cursorY = cursorRectangle.y

        if (getPostUtils().editMention.length > 0 && editMentionY != cursorY)
            getPostUtils().editMention = ""

        if (getPostUtils().editTag.length > 0 && editTagY != cursorY)
            getPostUtils().editTag = ""

        parentFlick.ensureVisible(cursorRectangle)
    }

    onTextChanged: {
        if (textChangeInProgress)
            return

        textChangeInProgress = true
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
        getPostUtils().setHighlightDocument(editText.textDocument, guiSettings.linkColor,
                                       editText.maxLength, guiSettings.textLengthExceededColor)
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

        const modifiedTillCursor = getPostUtils().applyFontToLastTypedChars(
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
        getPostUtils().extractMentionsAndLinks(editText.text,
                editText.preeditText, cursorPosition)
    }

    function updateGraphemeLength() {
        const prevGraphemeLength = graphemeLength
        const linkShorteningReduction = enableLinkShortening ? getPostUtils().getLinkShorteningReduction() : 0;

        graphemeLength = unicodeFonts.graphemeLength(editText.text) +
                unicodeFonts.graphemeLength(preeditText) -
                linkShorteningReduction

        getPostUtils().setHighLightMaxLength(editText.maxLength + linkShorteningReduction)

        return graphemeLength - prevGraphemeLength
    }

    Text {
        anchors.fill: parent
        leftPadding: editText.leftPadding
        rightPadding: editText.rightPadding
        topPadding: editText.topPadding
        bottomPadding: editText.bottomPadding
        font.pointSize: editText.font.pointSize
        color: guiSettings.placeholderTextColor
        text: placeholderText
        visible: editText.graphemeLength === 0
    }

    Timer {
        id: authorTypeaheadSearchTimer
        interval: 500
        onTriggered: {
            if (getPostUtils().editMention.length > 0)
                searchUtils.searchAuthorsTypeahead(getPostUtils().editMention, 10)
        }
    }

    Timer {
        id: hashtagTypeaheadSearchTimer
        interval: 500
        onTriggered: {
            if (getPostUtils().editTag.length > 0)
                searchUtils.searchHashtagsTypeahead(getPostUtils().editTag, 10)
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
    }

    UnicodeFonts {
        id: unicodeFonts
    }

    GuiSettings {
        id: guiSettings
    }

    function getPostUtils() {
        if (parentPostUtils)
            return parentPostUtils

        return postUtils
    }

    function startAuthorTypeaheadSearchTimer() {
        authorTypeaheadSearchTimer.start()
    }

    function startHashtagTypeaheadSearchTimer() {
        hashtagTypeaheadSearchTimer.start()
    }

    function createAuthorTypeaheadView() {
        let component = Qt.createComponent("AuthorTypeaheadView.qml")
        let page = component.createObject(parentPage, {
                parentPage: parentPage,
                editText: editText,
                searchUtils: searchUtils,
                postUtils: getPostUtils()
            })
    }

    function createHashtagTypeaheadView() {
        let component = Qt.createComponent("HashtagTypeaheadView.qml")
        let page = component.createObject(parentPage, {
                parentPage: parentPage,
                editText: editText,
                searchUtils: searchUtils,
                postUtils: getPostUtils()
            })
    }

    Component.onCompleted: {
        createAuthorTypeaheadView()
        createHashtagTypeaheadView()
        getPostUtils().setHighlightDocument(editText.textDocument, guiSettings.linkColor,
                                       editText.maxLength, guiSettings.textLengthExceededColor)
    }
}

