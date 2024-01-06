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

    EmptyListIndication {
        y: parent.headerItem ? parent.headerItem.height : 0
        svg: svgOutline.noLists
        text: qsTr("No lists")
        list: view
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
