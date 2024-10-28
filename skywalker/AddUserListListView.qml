pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import skywalker

ListView {
    required property var skywalker
    required property int modelId
    required property detailedprofile author

    signal closed

    id: view
    spacing: 0
    model: skywalker.getListListModel(modelId)
    flickDeceleration: guiSettings.flickDeceleration
    maximumFlickVelocity: guiSettings.maxFlickVelocity
    pixelAligned: guiSettings.flickPixelAligned
    clip: true
    ScrollIndicator.vertical: ScrollIndicator {}

    header: SimpleDescriptionHeader {
        title: qsTr("Update lists")
        description: qsTr(`Add/remove ${view.author.name}`)
        onClosed: view.closed()
    }
    headerPositioning: ListView.OverlayHeader

    delegate: AddUserListViewDelegate {
        width: view.width

        onAddToList: (listUri) => graphUtils.addListUser(listUri, view.author)
        onRemoveFromList: (listUri, listItemUri) => graphUtils.removeListUser(listUri, listItemUri)
    }

    FlickableRefresher {
        inProgress: view.skywalker.getListListInProgress
        bottomOvershootFun: () => view.skywalker.getListListNextPage(view.modelId)
    }

    EmptyListIndication {
        y: parent.headerItem ? parent.headerItem.height : 0
        svg: SvgOutline.noLists
        text: qsTr("No lists")
        list: view
    }

    BusyIndicator {
        anchors.centerIn: parent
        running: view.skywalker.getListListInProgress
    }

    GraphUtils {
        id: graphUtils
        skywalker: view.skywalker

        onAddListUserFailed: (error) => view.skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
        onRemoveListUserFailed: (error) => view.skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)

    }

    GuiSettings {
        id: guiSettings
    }

    function refresh() {
        skywalker.getListList(modelId)
    }

    Component.onDestruction: {
        skywalker.removeListListModel(modelId)
    }
}
