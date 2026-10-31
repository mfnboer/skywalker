import QtQuick
import QtQuick.Controls
import skywalker

SkyListView {
    required property string userDid
    property Skywalker skywalker: root.getSkywalker(userDid)
    property int modelId: -1
    required property var getListListFunc
    required property var getListListNextPageFunc
    required property GraphUtils graphUtils

    id: listView
    width: parent.width
    height: parent.height
    clip: true
    model: modelId >= 0 ? skywalker.getListListModel(modelId) : null
    preloadNextPageFunc: () => getListListNextPageFunc(modelId)

    delegate: ListViewDelegate {
        width: listView.width
        userDid: listView.userDid
        allowEdit: false

        onBlockList: (list) => graphUtils.blockList(list.uri)
        onUnblockList: (list, blockedUri) => graphUtils.unblockList(list.uri, blockedUri)
        onMuteList: (list) => graphUtils.muteList(list.uri)
        onUnmuteList: (list) => graphUtils.unmuteList(list.uri)
        onHideList: (list) => graphUtils.hideList(list.uri)
        onUnhideList: (list) => graphUtils.unhideList(list.uri)
        onHideReplies: (list, hide) => graphUtils.hideReplies(list.uri, hide)
        onHideFollowing: (list, hide) => graphUtils.hideFollowing(list.uri, hide)
        onSyncList: (list, sync) => graphUtils.syncList(list.uri, sync)
    }

    FlickableRefresher {
        inProgress: Boolean(listView.model?.getFeedInProgress)
        bottomOvershootFun: () => getListListNextPageFunc(modelId)
    }

    BusyIndicator {
        anchors.centerIn: parent
        running: Boolean(listView.model?.getFeedInProgress)
    }

    EmptyListIndication {
        svg: SvgOutline.noLists
        text: qsTr("No lists")
        list: listView
    }

    function refresh() {
        if (modelId >= 0)
            getListListFunc(modelId)
    }

    function clear() {
        model.clear()
    }
}