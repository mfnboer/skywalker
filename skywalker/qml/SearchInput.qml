import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

Rectangle {
    property string placeHolderText: qsTr("Search")
    property string prevDisplayText
    readonly property alias displayText: searchText.displayText
    readonly property alias cursorWord: searchText.cursorWord

    id: searchInput
    radius: guiSettings.radius
    implicitHeight: searchText.implicitHeight
    color: guiSettings.textInputBackgroundColor
    border.width: searchText.activeFocus ? 1 : 0
    border.color: guiSettings.buttonColor

    signal searchTextChanged(string text)
    signal search(string text)
    signal cleared()

    WordsInput {
        id: searchText
        EnterKey.type: Qt.EnterKeySearch // works since Qt6.8
        width: parent.width
        clip: true
        padding: 5
        rightPadding: clearButton.width
        font.pointSize: guiSettings.scaledFont(9/8)
        color: guiSettings.textColor
        inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
        maximumLength: 2048
        focus: true

        onDisplayTextChanged: {
            if (displayText !== prevDisplayText) {
                prevDisplayText = displayText
                searchTextChanged(displayText)
            }
        }

        Keys.onReturnPressed: search(searchText.displayText)

        Accessible.role: Accessible.EditableText
        Accessible.name: placeHolder.visible ? placeHolderText : text
        Accessible.editable: true
        Accessible.searchEdit: true

        SvgPlainButton {
            id: clearButton
            anchors.right: parent.right
            imageMargin: 8
            y: parent.y - parent.padding
            width: height
            height: parent.height + 10
            svg: SvgOutline.close
            Material.background: searchInput.color
            accessibleName: qsTr("clear search text")
            onClicked: {
                searchText.clear()
                cleared()
            }
        }
    }

    AccessibleText {
        id: placeHolder
        width: searchText.width
        padding: searchText.padding
        rightPadding: searchText.rightPadding
        font.pointSize: searchText.font.pointSize
        color: guiSettings.placeholderTextColor
        text: placeHolderText
        visible: searchText.displayText.length === 0

        Accessible.ignored: true
    }

    function setSearchText(text) {
        searchText.text = text
    }

    function setCursorWord(word) {
        searchText.replaceCursorWord(word)
    }

    function hasFocus() {
        return searchText.activeFocus
    }

    function forceFocus() {
        searchText.forceActiveFocus()
    }
}
