import QtQuick
import QtQuick.Controls
import skywalker

SkyListView {
    required property string userDid
    property Skywalker skywalker: root.getSkywalker(userDid)
    property int modelId: -1
    required property var getStarterPackListFunc
    required property var getStarterPackListNextPageFunc

    id: listView
    width: parent.width
    height: parent.height
    clip: true
    model: modelId >= 0 ? skywalker.getStarterPackListModel(modelId) : null
    preloadNextPageFunc: () => getStarterPackListNextPageFunc(modelId)

    delegate: StarterPackViewDelegate {
        width: listView.width
        userDid: listView.userDid
        allowEdit: false
    }

    FlickableRefresher {
        inProgress: Boolean(listView.model?.getFeedInProgress)
        bottomOvershootFun: () => getStarterPackListNextPageFunc(modelId)
    }

    BusyIndicator {
        anchors.centerIn: parent
        running: Boolean(listView.model?.getFeedInProgress)
    }

    EmptyListIndication {
        svg: SvgOutline.noLists
        text: qsTr("No starter packs")
        list: listView
    }

    function refresh() {
        getStarterPackListFunc(modelId)
    }

    function clear() {
        model.clear()
    }
}
