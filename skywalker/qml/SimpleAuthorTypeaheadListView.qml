import QtQuick
import skywalker

SimpleAuthorListView {
    required property string searchText
    property int searchLimit: 20
    property bool canChatOnly: false
    property Skywalker skywalker: root.getSkywalker()

    signal cleared

    id: view
    model: searchUtils.authorTypeaheadList

    Timer {
        id: authorTypeaheadSearchTimer
        interval: 500
        onTriggered: {
            if (searchText.length > 0) {
                searchUtils.searchAuthorsTypeahead(searchText, searchLimit, canChatOnly)
            } else {
                clear()
                cleared()
            }
        }
    }

    SearchUtils {
        id: searchUtils
        skywalker: view.skywalker
    }

    function startSearch() {
        authorTypeaheadSearchTimer.start()
    }

    function stopSearch() {
        authorTypeaheadSearchTimer.stop()
    }

    function clear() {
        searchUtils.authorTypeaheadList = []
    }

    function reset(authorList) {
        searchUtils.authorTypeaheadList = authorList
    }
}
