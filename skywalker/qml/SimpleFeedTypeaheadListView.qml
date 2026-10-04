import QtQuick
import skywalker

SimpleFeedListView {
    required property string searchText
    property int searchLimit: 50
    property Skywalker skywalker: root.getSkywalker()

    signal cleared

    id: view
    model: searchUtils.feedTypeaheadList

    Timer {
        id: feedTypeaheadSearchTimer
        interval: 500
        onTriggered: {
            if (searchText.length > 0) {
                searchUtils.searchFeedsTypeahead(searchText, searchLimit)
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
        feedTypeaheadSearchTimer.start()
    }

    function stopSearch() {
        feedTypeaheadSearchTimer.stop()
    }

    function clear() {
        searchUtils.feedTypeaheadList = []
        cleared()
    }

    function reset(feedList) {
        searchUtils.feedTypeaheadList = feedList
    }
}
