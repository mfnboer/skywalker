import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyListView {
    property string userDid
    required property string title
    property Skywalker skywalker: root.getSkywalker(userDid)
    required property int modelId
    property string description
    property bool showFollow: true
    property bool showActivitySubscription: false
    property bool allowDeleteItem: false
    property string listUri // set when the author list is a list of members from listUri
    property int prevModelId: -1
    readonly property string sideBarTitle: authorListView.title
    readonly property string sideBarDescription: authorListView.description
    readonly property SvgImage sideBarSvg: SvgOutline.group

    signal closed

    id: authorListView
    model: skywalker.getAuthorListModel(modelId)

    Accessible.role: Accessible.List

    header: Item {
        width: parent.width
        height: portraitHeader.visible ? portraitHeader.height : landscapeHeader.height
        z: guiSettings.headerZLevel

        SimpleDescriptionHeader {
            id: portraitHeader
            userDid: authorListView.userDid
            title: sideBarTitle
            description: sideBarDescription
            visible: authorListView.title && !root.showSideBar
            onClosed: authorListView.closed()
        }
    }
    headerPositioning: ListView.OverlayHeader

    delegate: AuthorViewDelegate {
        required property int index

        width: authorListView.width
        userDid: authorListView.userDid
        showFollow: authorListView.showFollow
        showActivitySubscription: authorListView.showActivitySubscription
        allowDeleteItem: authorListView.allowDeleteItem

        onFollow: (profile) => { graphUtils.follow(profile) }
        onUnfollow: (did, uri) => { graphUtils.unfollow(did, uri) }
        onDeleteItem: (listItemUri) => authorListView.deleteListItem(listItemUri, index)
    }

    onModelIdChanged: {
        if (prevModelId > -1) {
            console.debug("Delete previous model:", prevModelId)
            skywalker.removeAuthorListModel(prevModelId)
            prevModelId = modelId
            refresh()
        }
    }

    FlickableRefresher {
        inProgress: authorListView.model?.getFeedInProgress
        topOvershootFun: () => refresh()
        bottomOvershootFun: () => skywalker.getAuthorListNextPage(modelId)
        topText: qsTr("Refresh")
    }

    EmptyListIndication {
        y: parent.headerItem ? parent.headerItem.height : 0
        svg: SvgOutline.noUsers
        text: qsTr("None")
        list: authorListView
    }

    GraphUtils {
        id: graphUtils
        skywalker: authorListView.skywalker

        onFollowFailed: (error) => { skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR) }
        onUnfollowFailed: (error) => { skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR) }
        onRemoveListUserFailed: (error) => {
            skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
            refresh()
        }
    }

    BusyIndicator {
        id: busyBottomIndicator
        anchors.centerIn: parent
        running: authorListView.model?.getFeedInProgress
    }


    function deleteListItem(listItemUri, index) {
        model.deleteEntry(index)
        graphUtils.removeListUser(listUri, listItemUri)
    }

    function refresh() {
        skywalker.getAuthorList(modelId)
    }

    Component.onDestruction: {
        skywalker.removeAuthorListModel(modelId)
    }

    Component.onCompleted: {
        prevModelId = modelId
    }
}
