import QtQuick
import QtQuick.Controls
import skywalker

ListView {
    required property int modelId

    id: view
    spacing: 0
    model: skywalker.getPostThreadModel(modelId)
    ScrollIndicator.vertical: ScrollIndicator {}

    delegate: PostFeedViewDelegate {
        viewWidth: view.width
    }
}
