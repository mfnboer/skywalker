import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

HashtagListView {
    required property var parentPage
    required property var editText
    required property SearchUtils searchUtils
    required property PostUtils postUtils

    id: searchList
    y: getParentY(editText) + editText.cursorRectangle.y + editText.cursorRectangle.height + 5
    z: 10
    width: parentPage.width
    height: parentPage.footer.y - y - 5
    model: searchUtils.hashtagTypeaheadList
    visible: postUtils.editTag.length > 0

    onVisibleChanged: {
        if (!visible && searchUtils)
            searchUtils.hashtagTypeaheadList = []
    }

    onHashtagClicked: (hashtag) => {
        const {textBefore, textBetween, textAfter, fullText} = editText.getTextParts()
        const hashtagStartIndex = postUtils.getEditTagIndex()
        const hashtagEndIndex = hashtagStartIndex + postUtils.editTag.length
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
}
