import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

// Typeahead matches on parital mention
SimpleAuthorListView {
    required property var parentPage
    required property var editText
    required property SearchUtils searchUtils
    required property PostUtils postUtils

    id: typeaheadView
    y: getParentY(editText) + editText.cursorRectangle.y + editText.cursorRectangle.height + 5
    z: 10
    width: parentPage.width
    height: parentPage.footer.y - y - 5
    model: searchUtils.authorTypeaheadList
    visible: postUtils.editMention.length > 0

    onVisibleChanged: {
        if (!visible && searchUtils)
            searchUtils.authorTypeaheadList = []
    }

    onAuthorClicked: (profile) => {
        console.debug("AUTHOR CLICKED")
        const {textBefore, textBetween, textAfter, fullText} = editText.getTextParts()
        const mentionStartIndex = postUtils.getEditMentionIndex()
        const mentionEndIndex = mentionStartIndex + postUtils.editMention.length
        editText.clear() // also clears the preedit buffer

        // Add space and move the cursor 1 postion beyond the end
        // of the mention. That causes the typeahead list to disappear.
        const newText = fullText.slice(0, mentionStartIndex) + profile.handle + ' ' + fullText.slice(mentionEndIndex)
        editText.text = newText
        editText.cursorPosition = mentionStartIndex + profile.handle.length + 1
    }

    function getParentY(item) {
        return item.mapToItem(parentPage, 0, 0).y
    }
}
