import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

ListView {
    required property var parentPage
    required property var editText
    required property var searchUtils
    required property var postUtils

    signal hashtagClicked(string hashtag)

    id: searchList
    y: editText.mapToItem(parentPage, editText.x, editText.y).y + editText.cursorRectangle.y + editText.cursorRectangle.height + 5
    z: 10
    width: parentPage.width
    height: parentPage.footer.y - y - 5
    model: searchUtils.hashtagTypeaheadList
    spacing: 0
    boundsBehavior: Flickable.StopAtBounds
    clip: true
    flickDeceleration: guiSettings.flickDeceleration
    visible: postUtils.editTag.length > 0

    Accessible.role: Accessible.List

    onVisibleChanged: {
        if (!visible)
            searchUtils.hashtagTypeaheadList = []
    }

    delegate: Rectangle {
        required property string modelData
        property alias hashtag: hashtagEntry.modelData

        id: hashtagEntry
        width: searchList.width
        height: hashtagText.height
        color: guiSettings.backgroundColor

        Accessible.role: Accessible.Button
        Accessible.name: hashtag
        Accessible.onPressAction: hashtagClicked(hashtagEntry.hashtag)

        SkyCleanedText {
            id: hashtagText
            width: parent.width
            padding: 10
            elide: Text.ElideRight
            font.bold: true
            color: guiSettings.textColor
            plainText: `#${hashtagEntry.hashtag}`

            Accessible.ignored: true
        }

        Rectangle {
            width: parent.width
            height: 1
            color: guiSettings.separatorColor
        }

        MouseArea {
            z: -1
            anchors.fill: parent
            onClicked: hashtagClicked(hashtagEntry.hashtag)
        }
    }

    // TODO: almost same as onAuthorClicking in AuthorTypeaheadView
    onHashtagClicked: (hashtag) => {
        const textBefore = editText.text.slice(0, editText.cursorPosition)
        const textBetween = editText.preeditText
        const textAfter = editText.text.slice(editText.cursorPosition)
        const fullText = textBefore + textBetween + textAfter
        const hashtagStartIndex = postUtils.getEditTagIndex()
        const hashtagEndIndex = hashtagStartIndex + postUtils.editTag.length
        editText.clear() // also clears the preedit buffer

        // Add space and move the cursor 1 postion beyond the end
        // of the tag. That causes the typeahead list to disappear.
        const newText = fullText.slice(0, hashtagStartIndex) + hashtag + ' ' + fullText.slice(hashtagEndIndex)
        editText.text = newText
        editText.cursorPosition = hashtagStartIndex + hashtag.length + 1
    }
}
