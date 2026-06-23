import QtQuick
import skywalker

SimpleAuthorListView {
    required property string searchText
    property int searchLimit: 20
    property int searchFilter: QEnums.AUTHOR_SEARCH_FILTER_NONE
    property bool publicSearch: false
    property Skywalker skywalker: root.getSkywalker()

    signal cleared

    id: view
    model: searchUtils.authorTypeaheadList

    Timer {
        id: authorTypeaheadSearchTimer
        interval: 500
        onTriggered: {
            if (searchText.length > 0) {
                if (publicSearch)
                    searchUtils.publicSearchAuthorsTypeahead(searchText, searchLimit)
                else
                    searchUtils.searchAuthorsTypeahead(searchText, searchLimit, searchFilter)
            } else {
                clear()
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
        cleared()
    }

    function reset(authorList) {
        searchUtils.authorTypeaheadList = authorList
    }
}
