import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyListView {
    property Skywalker skywalker: root.getSkywalker()
    required property int modelId
    property string description
    property bool ownLists: true
    property var timelineHide: skywalker.getTimelineHide()

    signal closed

    id: view
    spacing: 0
    model: skywalker.getListListModel(modelId)
    clip: true

    Accessible.role: Accessible.List

    header: Rectangle {
        width: parent.width
        height: headerRow.height
        z: guiSettings.headerZLevel
        color: guiSettings.backgroundColor

        RowLayout {
            id: headerRow
            width: parent.width

            AccessibleText {
                Layout.fillWidth: true
                padding: 10
                wrapMode: Text.Wrap
                elide: Text.ElideRight
                color: guiSettings.textColor
                text: view.description
            }

            SvgPlainButton {
                id: addButton
                svg: SvgOutline.add
                accessibleName: qsTr("create new list")
                visible: ownLists
                onClicked: root.newList(view.model)
            }
        }
    }
    headerPositioning: ListView.OverlayHeader

    delegate: ListViewDelegate {
        required property int index

        width: view.width

        onUpdateList: (list) => view.editList(list, index)
        onDeleteList: (list) => view.deleteList(list, index)
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
        inProgress: view.model?.getFeedInProgress
        topOvershootFun: () => refresh()
        bottomOvershootFun: () => skywalker.getListListNextPage(modelId)
        topText: qsTr("Refresh lists")
    }

    EmptyListIndication {
        y: parent.headerItem ? parent.headerItem.height : 0
        svg: SvgOutline.noLists
        text: qsTr("No lists")
        list: view
    }

    BusyIndicator {
        anchors.centerIn: parent
        running: view.model?.getFeedInProgress
    }

    GraphUtils {
        id: graphUtils
        skywalker: view.skywalker // qmllint disable missing-type

        onDeleteListFailed: (error) => {
            skywalker.showStatusMessage(qsTr(`Failed to delete list: ${error}`), QEnums.STATUS_LEVEL_ERROR)
            skywalker.getListList(modelId)
        }

        onBlockListFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
        onUnblockListFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
        onMuteListFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
        onUnmuteListFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
        onHideListOk: skywalker.showStatusMessage(qsTr("List hidden from timeline."), QEnums.STATUS_LEVEL_INFO, 2)
        onHideListFailed: (error) => skywalker.showStatusMessage(qsTr(`Failed to hide list from timeline: ${error}`), QEnums.STATUS_LEVEL_INFO, 2)
    }

    function editList(list, index) {
        let component = guiSettings.createComponent("EditList.qml")
        let page = component.createObject(view, {
                skywalker: skywalker,
                purpose: list.purpose,
                list: list
            })
        page.onListUpdated.connect((cid, name, description, embeddedLinks, avatar) => {
            skywalker.showStatusMessage(qsTr("List updated."), QEnums.STATUS_LEVEL_INFO, 2)
            let oldList = view.model.getEntry(index)
            let newList = view.model.updateEntry(index, cid, name, description, embeddedLinks, avatar)

            if (skywalker.favoriteFeeds.isPinnedFeed(oldList.uri)) {
                skywalker.favoriteFeeds.removeList(oldList)

                if (!newList.isNull())
                    skywalker.favoriteFeeds.pinList(newList, true)
            }

            root.popStack()
        })
        page.onClosed.connect(() => { root.popStack() })
        root.pushStack(page)
    }

    function deleteList(list, index) {
        guiSettings.askYesNoQuestion(
                    view,
                    qsTr(`Do you really want to delete: ${(list.name)} ?`),
                    () => view.continueDeleteList(list, index))
    }

    function continueDeleteList(list, index) {
        view.model.deleteEntry(index)
        skywalker.favoriteFeeds.removeList(list)
        graphUtils.deleteList(list.uri)
    }

    function refresh() {
        skywalker.getListList(modelId)
    }

    Component.onDestruction: {
        skywalker.removeListListModel(modelId)
    }
}
