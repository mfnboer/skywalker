import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

Rectangle {
    property string placeHolderText: qsTr("Search")
    property bool showBackButton: true
    readonly property alias displayText: searchInput.displayText
    readonly property alias cursorWord: searchInput.cursorWord

    signal back
    signal searchTextChanged(string text)
    signal search(string text)
    signal cleared()

    id: headerRect
    width: parent.width
    height: guiSettings.headerHeight
    z: guiSettings.headerZLevel
    color: guiSettings.headerColor

    RowLayout {
        width: parent.width
        height: guiSettings.headerHeight

        SvgPlainButton {
            id: backButton
            iconColor: guiSettings.headerTextColor
            svg: SvgOutline.arrowBack
            accessibleName: qsTr("go back")
            visible: showBackButton
            onClicked: headerRect.back()
        }

        SearchInput {
            id: searchInput
            Layout.fillWidth: true
            Layout.rightMargin: 10
            placeHolderText: headerRect.placeHolderText

            onSearchTextChanged: (text) => headerRect.searchTextChanged(text)
            onSearch: (text) => headerRect.search(text)
            onCleared: headerRect.cleared()
        }
    }


    function setSearchText(text) {
        searchInput.setSearchText(text)
    }

    function setCursorWord(word) {
        searchInput.setCursorWord(word)
    }

    function hasFocus() {
        return searchInput.hasFocus()
    }

    function forceFocus() {
        searchInput.forceFocus()
    }

    function unfocus() {
        // Hack to hide the selection anchor on Android that should not have been
        // there at all.
        backButton.forceActiveFocus()
    }
}
