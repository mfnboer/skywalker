import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

// Typeahead matches on parital mention
SimpleAuthorListView {
    required property var parentPage
    required property var editText
    required property var searchUtils
    required property var postUtils

    id: typeaheadView
    y: editText.mapToItem(parentPage, editText.x, editText.y).y + editText.cursorRectangle.y + editText.cursorRectangle.height + 5
    z: 10
    width: parentPage.width
    height: parentPage.footer.y - y - 5
    model: searchUtils.authorTypeaheadList
    visible: postUtils.editMention.length > 0

    onVisibleChanged: {
        if (!visible)
            searchUtils.authorTypeaheadList = []
    }

    onAuthorClicked: (profile) => {
        const textBefore = editText.text.slice(0, editText.cursorPosition)
        const textBetween = editText.preeditText
        const textAfter = editText.text.slice(editText.cursorPosition)
        const fullText = textBefore + textBetween + textAfter
        const mentionStartIndex = postUtils.getEditMentionIndex()
        const mentionEndIndex = mentionStartIndex + postUtils.editMention.length
        editText.clear() // also clears the preedit buffer

        // Add space and move the cursor 1 postion beyond the end
        // of then mention. That causes the typeahead list to disappear.
        const newText = fullText.slice(0, mentionStartIndex) + profile.handle + ' ' + fullText.slice(mentionEndIndex)
        editText.text = newText
        editText.cursorPosition = mentionStartIndex + profile.handle.length + 1
    }
}
