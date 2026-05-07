import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyPage {
    required property var skywalker
    property bool isTyping: true

    signal closed
    signal authorClicked(basicprofile profile)

    id: page

    header: SearchHeader {
        placeHolderText: qsTr("Search user")
        showSearchButton: false

        onBack: page.closed()

        onSearchTextChanged: (text) => {
            page.isTyping = true

            if (text.length > 0) {
                typeaheadView.startSearch()
            } else {
                typeaheadView.stopSearch()
                typeaheadView.clear()
            }
        }
    }

    SimpleAuthorTypeaheadListView {
        id: typeaheadView
        anchors.fill: parent
        searchText: page.header.displayText
        onAuthorClicked: (profile) => page.authorClicked(profile)

        AccessibleText {
            topPadding: 10
            anchors.horizontalCenter: parent.horizontalCenter
            color: Material.color(Material.Grey)
            elide: Text.ElideRight
            text: qsTr("No matching user name found")
            visible: typeaheadView.count === 0
        }
    }

    function forceDestroy() {
        destroy()
    }

    function hide() {
        page.header.unfocus()
    }

    function show() {
        page.header.forceFocus()
    }
}
