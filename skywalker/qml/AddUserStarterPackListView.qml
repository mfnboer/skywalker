import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

ListView {
    property string userDid
    property Skywalker skywalker: root.getSkywalker(userDid)
    required property int modelId
    required property detailedprofile author
    readonly property string sideBarTitle: qsTr("Update starter packs")
    readonly property string sideBarDescription: qsTr(`Add/remove ${author.name}`)

    signal closed

    id: view
    spacing: 0
    model: skywalker.getStarterPackListModel(modelId)
    flickDeceleration: guiSettings.flickDeceleration
    maximumFlickVelocity: guiSettings.maxFlickVelocity
    pixelAligned: guiSettings.flickPixelAligned
    clip: true
    ScrollIndicator.vertical: ScrollIndicator {}

    header: Item {
        width: parent.width
        height: portraitHeader.visible ? portraitHeader.height : 0
        z: guiSettings.headerZLevel

        SimpleDescriptionHeader {
            id: portraitHeader
            userDid: view.userDid
            title: sideBarTitle
            description: sideBarDescription
            visible: !root.showSideBar
            onClosed: view.closed()
        }
    }
    headerPositioning: ListView.OverlayHeader

    delegate: AddUserStarterPackViewDelegate {
        width: view.width

        onAddToStarterPack: (starterPack) => graphUtils.addListUser(starterPack.list.uri, author)
        onRemoveFromStarterPack: (starterPack, listItemUri) => graphUtils.removeListUser(starterPack.list.uri, listItemUri)
    }

    FlickableRefresher {
        inProgress: view.model?.getFeedInProgress
        topOvershootFun: () => skywalker.getAuthorStarterPackList(userDid, modelId)
        bottomOvershootFun: () => skywalker.getAuthorStarterPackListNextPage(userDid, modelId)
        topText: qsTr("Refresh starter packs")
    }

    EmptyListIndication {
        y: parent.headerItem ? parent.headerItem.height : 0
        svg: SvgOutline.noLists
        text: qsTr("No starter packs")
        list: view
    }

    BusyIndicator {
        anchors.centerIn: parent
        running: view.model?.getFeedInProgress
    }

    GraphUtils {
        id: graphUtils
        skywalker: view.skywalker

        onAddListUserFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
        onRemoveListUserFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
    }

    function refresh() {
        skywalker.getAuthorStarterPackList(userDid, modelId)
    }

    Component.onDestruction: {
        skywalker.removeStarterPackListModel(modelId)
    }
}
