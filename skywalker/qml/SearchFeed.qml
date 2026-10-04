import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyPage {
    required property Skywalker skywalker
    property list<generatorview> presetTypeaheadList: []
    property bool isTyping: true

    signal closed
    signal feedClicked(generatorview feed)

    id: page

    header: SearchHeader {
        placeHolderText: qsTr("Search feed")
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

    SimpleFeedTypeaheadListView {
        id: typeaheadView
        anchors.fill: parent
        searchText: page.header.displayText

        onFeedClicked: (feed) => page.feedClicked(feed)
        onCleared: resetFeedTypeaheadList()

        AccessibleText {
            topPadding: 10
            anchors.horizontalCenter: parent.horizontalCenter
            color: Material.color(Material.Grey)
            elide: Text.ElideRight
            text: qsTr("No matching feed found")
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

    function resetFeedTypeaheadList() {
        typeaheadView.reset(presetTypeaheadList)
    }

    Component.onCompleted: {
        resetFeedTypeaheadList()
    }
}
