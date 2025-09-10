import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

ListView {
    required property var skywalker
    required property int modelId
    required property detailedprofile author
    readonly property string sideBarTitle: qsTr("Update lists")
    readonly property string sideBarDescription: qsTr(`Add/remove ${author.name}`)

    signal closed

    id: view
    spacing: 0
    model: skywalker.getListListModel(modelId)
    flickDeceleration: guiSettings.flickDeceleration
    maximumFlickVelocity: guiSettings.maxFlickVelocity
    pixelAligned: guiSettings.flickPixelAligned
    clip: true
    ScrollIndicator.vertical: ScrollIndicator {}

    header: Item {
        width: parent.width
        height: portraitHeader.visible ? portraitHeader.height : landscapeHeader.height
        z: guiSettings.headerZLevel

        SimpleDescriptionHeader {
            id: portraitHeader
            title: sideBarTitle
            description: sideBarDescription
            visible: !root.showSideBar
            onClosed: view.closed()
        }
        DeadHeaderMargin {
            id: landscapeHeader
            visible: root.showSideBar
        }
    }
    headerPositioning: ListView.OverlayHeader

    delegate: AddUserListViewDelegate {
        width: view.width

        onAddToList: (listUri) => graphUtils.addListUser(listUri, author)
        onRemoveFromList: (listUri, listItemUri) => graphUtils.removeListUser(listUri, listItemUri)
    }

    FlickableRefresher {
        inProgress: skywalker.getListListInProgress
        bottomOvershootFun: () => skywalker.getListListNextPage(modelId)
    }

    EmptyListIndication {
        y: parent.headerItem ? parent.headerItem.height : 0
        svg: SvgOutline.noLists
        text: qsTr("No lists")
        list: view
    }

    BusyIndicator {
        anchors.centerIn: parent
        running: skywalker.getListListInProgress
    }

    GraphUtils {
        id: graphUtils
        skywalker: view.skywalker

        onAddListUserFailed: (error) => statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
        onRemoveListUserFailed: (error) => statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)

    }


    function refresh() {
        skywalker.getListList(modelId)
    }

    Component.onDestruction: {
        skywalker.removeListListModel(modelId)
    }
}
