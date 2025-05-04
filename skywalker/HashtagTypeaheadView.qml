import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

HashtagListView {
    required property var parentPage
    required property var editText
    required property SearchUtils searchUtils
    required property FacetUtils facetUtils

    id: searchList
    z: 10
    width: parentPage.width
    model: searchUtils ? searchUtils.hashtagTypeaheadList : undefined
    visible: facetUtils && facetUtils.editTag.length > 0

    onVisibleChanged: {
        if (!visible && searchUtils)
            searchUtils.hashtagTypeaheadList = []

        if (visible)
            setPosition()
    }

    onHashtagClicked: (hashtag) => {
        const {textBefore, textBetween, textAfter, fullText} = editText.getTextParts()
        const hashtagStartIndex = facetUtils.getEditTagIndex()
        const hashtagEndIndex = hashtagStartIndex + facetUtils.editTag.length
        editText.clear() // also clears the preedit buffer

        // Add space and move the cursor 1 postion beyond the end
        // of the tag. That causes the typeahead list to disappear.
        const newText = fullText.slice(0, hashtagStartIndex) + hashtag + ' ' + fullText.slice(hashtagEndIndex)
        editText.text = newText
        editText.cursorPosition = hashtagStartIndex + hashtag.length + 1
    }

    function getParentY(item) {
        return item.mapToItem(parentPage, 0, 0).y
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
