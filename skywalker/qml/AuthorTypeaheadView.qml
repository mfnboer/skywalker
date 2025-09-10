import QtQuick
import skywalker

// Typeahead matches on parital mention
SimpleAuthorListView {
    required property var parentPage
    required property var editText
    required property SearchUtils searchUtils
    required property FacetUtils facetUtils

    id: typeaheadView
    z: 10
    width: parentPage.width
    model: searchUtils ? searchUtils.authorTypeaheadList : undefined
    visible: facetUtils && facetUtils.editMention.length > 0

    onVisibleChanged: {
        if (!visible && searchUtils)
            searchUtils.authorTypeaheadList = []

        if (visible)
            setPosition()
    }

    onAuthorClicked: (profile) => {
        const {textBefore, textBetween, textAfter, fullText} = editText.getTextParts()
        const mentionStartIndex = facetUtils.getEditMentionIndex()
        const mentionEndIndex = mentionStartIndex + facetUtils.editMention.length
        editText.clear() // also clears the preedit buffer

        // Add space and move the cursor 1 postion beyond the end
        // of the mention. That causes the typeahead list to disappear.
        const newText = fullText.slice(0, mentionStartIndex) + profile.handle + ' ' + fullText.slice(mentionEndIndex)
        editText.text = newText
        editText.cursorPosition = mentionStartIndex + profile.handle.length + 1
    }

    function setPosition() {
        const belowY = editText.mapToItem(parentPage, 0, 0).y + editText.cursorRectangle.y + editText.cursorRectangle.height + 5
        const belowHeight = parentPage.footer.y - belowY - 5

        if (belowHeight > 80) {
            y = belowY
            height = belowHeight
            return
        }

        const aboveY = editText.mapToItem(parentPage, 0, 0).y + editText.cursorRectangle.y - 5
        const aboveHeight = aboveY - (parentPage.header ? parentPage.header.height : 0) - 5
        y = aboveY - aboveHeight
        height = aboveHeight
    }
}
