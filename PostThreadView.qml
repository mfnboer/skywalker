import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

ListView {
    required property int modelId
    required property int postEntryIndex
    signal closed

    id: view
    spacing: 0
    model: skywalker.getPostThreadModel(modelId)
    ScrollIndicator.vertical: ScrollIndicator {}

    header: Rectangle {
        width: parent.width
        height: backButton.height
        z: 10
        color: "white"
        border.width: 1

        SvgButton {
            id: backButton
            iconColor: Material.foreground
            Material.background: "transparent"
            svg: svgOutline.arrowBack
            onClicked: view.closed()
        }
        Text {
            id: headerText
            height: backButton.height
            anchors.left: backButton.right
            font.bold: true
            text: qsTr("Post thread")
        }
    }
    headerPositioning: ListView.OverlayHeader

    delegate: PostFeedViewDelegate {
        viewWidth: view.width
    }

    Component.onCompleted: {
        console.debug("Entry index:", postEntryIndex);
        positionViewAtIndex(postEntryIndex, ListView.Beginning)
    }
    Component.onDestruction: skywalker.removePostThreadModel(modelId)
}
