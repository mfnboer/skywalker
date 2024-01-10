import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Page {
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
                typeaheadSearchTimer.start()
            } else {
                typeaheadSearchTimer.stop()
                searchUtils.authorTypeaheadList = []
            }
        }
    }

    SimpleAuthorListView {
        id: typeaheadView
        anchors.fill: parent
        model: searchUtils.authorTypeaheadList
        onAuthorClicked: (profile) => page.authorClicked(profile)

        Text {
            topPadding: 10
            anchors.horizontalCenter: parent.horizontalCenter
            color: Material.color(Material.Grey)
            elide: Text.ElideRight
            text: qsTr("No matching user name found")
            visible: typeaheadView.count === 0
        }
    }

    Timer {
        id: typeaheadSearchTimer
        interval: 500
        onTriggered: {
            const text = page.header.getDisplayText()

            if (text.length > 0)
                searchUtils.searchAuthorsTypeahead(text)
        }
    }

    SearchUtils {
        id: searchUtils
        skywalker: page.skywalker
    }

    GuiSettings {
        id: guiSettings
    }

    function forceDestroy() {
        searchUtils.clearAllSearchResults();
        destroy()
    }

    function hide() {
        page.header.unfocus()
    }

    function show() {
        page.header.forceFocus()
    }
}
