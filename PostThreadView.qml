import QtQuick
import QtQuick.Controls
import skywalker

ListView {
    required property int modelId
    required property int postEntryIndex

    id: view
    spacing: 0
    model: skywalker.getPostThreadModel(modelId)
    ScrollIndicator.vertical: ScrollIndicator {}

    delegate: PostFeedViewDelegate {
        viewWidth: view.width
    }

    Component.onCompleted: {
        console.debug("Entry index:", postEntryIndex);
        positionViewAtIndex(postEntryIndex, ListView.Beginning)
    }
    Component.onDestruction: skywalker.removePostThreadModel(modelId)
}
