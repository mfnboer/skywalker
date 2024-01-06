import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

ListView {
    required property string title
    required property var skywalker
    required property int modelId
    property string description

    signal closed

    id: view
    spacing: 0
    model: skywalker.getListListModel(modelId)
    flickDeceleration: guiSettings.flickDeceleration
    clip: true
    ScrollIndicator.vertical: ScrollIndicator {}

    header: SimpleDescriptionHeader {
        title: view.title
        description: view.description
        onClosed: view.closed()

        SvgButton {
            anchors.right: parent.right
            anchors.top: parent.top
            svg: svgOutline.add
        }
    }
    headerPositioning: ListView.OverlayHeader

    delegate: ListViewDelegate {
        viewWidth: view.width
    }

    FlickableRefresher {
        inProgress: skywalker.getListListInProgress
        verticalOvershoot: view.verticalOvershoot
        topOvershootFun: () => skywalker.getListList(modelId)
        bottomOvershootFun: () => skywalker.getListListNextPage(modelId)
        topText: qsTr("Refresh lists")
    }

    SvgImage {
        id: noListsImage
        width: 150
        height: 150
        y: height + (parent.headerItem ? parent.headerItem.height : 0)
        anchors.horizontalCenter: parent.horizontalCenter
        color: Material.color(Material.Grey)
        svg: svgOutline.noLists
        visible: view.count === 0
    }
    Text {
        id: noListsText
        y: noListsImage.y
        anchors.horizontalCenter: parent.horizontalCenter
        font.pointSize: guiSettings.scaledFont(10/8)
        color: Material.color(Material.Grey)
        elide: Text.ElideRight
        text: qsTr("No lists")
        visible: view.count === 0
    }

    BusyIndicator {
        anchors.centerIn: parent
        running: skywalker.getListListInProgress
    }

    GuiSettings {
        id: guiSettings
    }

    Component.onDestruction: {
        skywalker.removeListListModel(modelId)
    }
}
