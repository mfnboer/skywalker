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
    property string textWithoutLinks: ""
    property string firstWebLink
    property bool cursorInFirstWebLink: false
    property string firstPostLink
    property bool cursorInFirstPostLink: false
    property string firstFeedLink
    property bool cursorInFirstFeedLink: false
    property string firstListLink
    property bool cursorInFirstListLink: false
    property list<string> mentions
    property list<weblink> webLinks
    property int cursorInWebLink: -1
    property list<weblink> embeddedLinks
    property int cursorInEmbeddedLink: -1
    property string prevText: ""
    property int prevTextLen: 0
    property int lastDeltaTextLen: 0
    property bool suppressTextUpdates: false

    signal textUpdated

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
        let editMentionY = facetUtils.editMentionCursorY
        let editTagY = facetUtils.editTagCursorY
        let cursorY = cursorRectangle.y

        if (facetUtils.editMention.length > 0 && editMentionY !== cursorY)
            facetUtils.editMention = ""

        if (facetUtils.editTag.length > 0 && editTagY !== cursorY)
            facetUtils.editTag = ""

        ensureVisible(cursorRectangle)
    }

    onCursorPositionChanged: facetUtils.updateCursor(cursorPosition)

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

        facetUtils.updateText(prevText, text)
        prevText = text

        highlightFacets()

        const added = updateGraphemeLength()

        if (added > 0)
            updateTextTimer.set(added)

        textChangeInProgress = false

        if (!suppressTextUpdates)
            textUpdated()
    }

    onPreeditTextChanged: {
        if (textChangeInProgress)
            return

        const added = updateGraphemeLength()

        if (added > 0)
            updateTextTimer.set(added)
    }

    onMaxLengthChanged: {
        facetUtils.setHighlightDocument(
                editText.textDocument, guiSettings.linkColor, guiSettings.errorColor,
                editText.maxLength, guiSettings.textLengthExceededColor)
    }

    // HACK:
    // Sometimes while editing and moving the cursor by tapping on the screen, the text
    // sticks to the cursor, or the cursor begins to jump when typing. The reset on tap
    // seems to prevent this.
    // An alternative with TapHandler did not work
    // Can we ditch this with Qt6.8.2?
    // Note: Qt.inputMethod.visible will not work with softInputMode:AdjustNothing
    // MouseArea {
    //     anchors.fill: parent
    //     propagateComposedEvents: true
    //     onPressed: (mouse) => handleEvent(mouse)
    //     onPressAndHold: (mouse) => { mouse.accepted = false }
    //     onReleased: (mouse) => { mouse.accepted = false }
    //     onClicked: (mouse) => handleEvent(mouse)
    //     onDoubleClicked: (mouse) => handleEvent(mouse)

    //     function handleEvent(mouse) {
    //         if (Qt.inputMethod.visible) { // qmllint disable missing-property
    //             Qt.inputMethod.reset() // qmllint disable missing-property
    //             // editText.forceActiveFocus()
    //             // let position = editText.positionAt(mouse.x, mouse.y)
    //             // editText.cursorPosition = position
    //             Qt.inputMethod.show()
    //         }

    //         // Pass to TextEdit to handle the mouse event. This seems
    //         // better than setting the position ourselves.
    //         mouse.accepted = false
    //     }
    // }

    // Avoid keyboard popping up when the user is scrolling the parent flick
    MouseArea {
        anchors.fill: parent
        propagateComposedEvents: true

        onPressed: (mouse) => {
            console.debug("Text pressed")
            mouse.accepted = !editText.selectedText
        }

        // This signal only comes in if onPressed was accepted.
        onClicked: (mouse) => {
            console.debug("Text clicked")

            if (!editText.selectedText) {
                Qt.inputMethod.reset()
                editText.forceActiveFocus()
                let position = editText.positionAt(mouse.x, mouse.y)
                editText.cursorPosition = position
                Qt.inputMethod.show()
            }

            mouse.accepted = false
        }
    }

    // Text can only be changed outside onPreeditTextChanged.
    // This timer makes the call to applyFont async.
    Timer {
        property int numChars: 1

        id: updateTextTimer
        interval: 0
        onTriggered: {
            editText.textChangeInProgress = true
            const changesMade = editText.applyFont(numChars)
            editText.textChangeInProgress = false

            if (changesMade)
                editText.textUpdated()
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
            return false

        const modifiedTillCursor = facetUtils.applyFontToLastTypedChars(
                                     editText.text, editText.preeditText,
                                     editText.cursorPosition, numChars,
                                     fontSelectorCombo.currentIndex)

        if (modifiedTillCursor) {
            const fullText = modifiedTillCursor + editText.text.slice(editText.cursorPosition)
            editText.clear()
            editText.text = fullText
            editText.cursorPosition = modifiedTillCursor.length
            return true
        }

        return false
    }

    function highlightFacets() {
        facetUtils.extractMentionsAndLinks(editText.text,
                editText.preeditText, cursorPosition)
    }

    function updateGraphemeLength() {
        const prevGraphemeLength = graphemeLength
        const linkShorteningReduction = enableLinkShortening ? facetUtils.getLinkShorteningReduction() : 0

        graphemeLength = UnicodeFonts.graphemeLength(editText.text) +
                UnicodeFonts.graphemeLength(preeditText) -
                linkShorteningReduction

        facetUtils.setHighLightMaxLength(editText.maxLength + linkShorteningReduction)

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
            if (facetUtils.editMention.length > 0)
                searchUtils.searchAuthorsTypeahead(facetUtils.editMention, 10)
        }
    }

    Timer {
        id: hashtagTypeaheadSearchTimer
        interval: 500
        onTriggered: {
            if (facetUtils.editTag.length > 0)
                searchUtils.searchHashtagsTypeahead(facetUtils.editTag, 10)
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

    FacetUtils {
        property double editMentionCursorY: 0
        property double editTagCursorY: 0

        id: facetUtils
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

        onTextWithoutLinksChanged: editText.textWithoutLinks = textWithoutLinks;
        onFirstWebLinkChanged: editText.firstWebLink = firstWebLink
        onCursorInFirstWebLinkChanged: editText.cursorInFirstWebLink = cursorInFirstWebLink
        onFirstPostLinkChanged: editText.firstPostLink = firstPostLink
        onCursorInFirstPostLinkChanged: editText.cursorInFirstPostLink = cursorInFirstPostLink
        onFirstFeedLinkChanged: editText.firstFeedLink = firstFeedLink
        onCursorInFirstFeedLinkChanged: editText.cursorInFirstFeedLink = cursorInFirstFeedLink
        onFirstListLinkChanged: editText.firstListLink = firstListLink
        onCursorInFirstListLinkChanged: editText.cursorInFirstListLink = cursorInFirstListLink
        onMentionsChanged: editText.mentions = mentions
        onWebLinksChanged: editText.webLinks = webLinks
        onCursorInWebLinkChanged: editText.cursorInWebLink = cursorInWebLink
        onEmbeddedLinksChanged: editText.embeddedLinks = embeddedLinks
        onCursorInEmbeddedLinkChanged: editText.cursorInEmbeddedLink = cursorInEmbeddedLink
    }

    function maxGraphemeLengthExceeded() {
        return maxLength > -1 && graphemeLength > maxLength
    }

    function getTextParts() {
        const textBefore = editText.text.slice(0, editText.cursorPosition)
        const textBetween = editText.preeditText
        const textAfter = editText.text.slice(editText.cursorPosition)
        return {textBefore, textBetween, textAfter}
    }

    function getFullText() {
        const {textBefore, textBetween, textAfter} = getTextParts()
        const fullText = textBefore + textBetween + textAfter
        return fullText
    }

    function replaceLinkWithName(link, name)
    {
        const textBefore = editText.text.slice(0, link.startIndex)
        const textAfter = editText.text.slice(link.endIndex)
        const newText = textBefore + name + textAfter
        editText.text = newText
    }

    function makeEmbeddedLink(name, oldLink)
    {
        const embedStart = oldLink.startIndex
        const embedEnd = embedStart + name.length
        return facetUtils.makeWebLink(name, oldLink.link, embedStart, embedEnd)
    }

    function addEmbeddedLink(webLinkIndex, name) {
        // Delay text updates
        // As links get replaced by names in the text, the text may exceed the maximum
        // length. That would trigger a split before the new link has been added.
        // Adding the link before doing text replacements goes wrong as text replacement
        // will cause link shifts.
        suppressTextUpdates = true

        const webLink = facetUtils.webLinks[webLinkIndex]
        replaceLinkWithName(webLink, name)
        const embeddedLink = makeEmbeddedLink(name, webLink)
        facetUtils.addEmbeddedLink(embeddedLink)

        suppressTextUpdates = false

        textUpdated()
        cursorPosition = embeddedLink.endIndex
    }

    function updateEmbeddedLink(linkIndex, name)
    {
        suppressTextUpdates = true

        const embeddedLink = facetUtils.embeddedLinks[linkIndex]
        replaceLinkWithName(embeddedLink, name)
        const updatedLink = makeEmbeddedLink(name, embeddedLink)
        facetUtils.updatedEmbeddedLink(linkIndex, updatedLink)

        suppressTextUpdates = false

        textUpdated()
        cursorPosition = updatedLink.endIndex
    }

    function removeEmbeddedLink(linkIndex) {
        const embeddedLink = facetUtils.embeddedLinks[linkIndex]
        const rawLink = embeddedLink.link
        facetUtils.removeEmbeddedLink(linkIndex)
        replaceLinkWithName(embeddedLink, rawLink)
        cursorPosition = embeddedLink.startIndex + rawLink.length
    }

    function setEmbeddedLinks(embeddedLinkList) {
        facetUtils.embeddedLinks = embeddedLinkList
    }

    function checkMisleadingEmbeddedLinks() {
        return facetUtils.checkMisleadingEmbeddedLinks()
    }

    function createAuthorTypeaheadView() {
        let component = guiSettings.createComponent("AuthorTypeaheadView.qml")
        let page = component.createObject(parentPage, {
                parentPage: parentPage,
                editText: editText,
                searchUtils: searchUtils,
                facetUtils: facetUtils
            })
    }

    function createHashtagTypeaheadView() {
        let component = guiSettings.createComponent("HashtagTypeaheadView.qml")
        let page = component.createObject(parentPage, {
                parentPage: parentPage,
                editText: editText,
                searchUtils: searchUtils,
                facetUtils: facetUtils
            })
    }

    Component.onCompleted: {
        createAuthorTypeaheadView()
        createHashtagTypeaheadView()
        facetUtils.setHighlightDocument(
                editText.textDocument, guiSettings.linkColor, guiSettings.errorColor,
                editText.maxLength, guiSettings.textLengthExceededColor)
    }
}

