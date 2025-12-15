import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

ListView {
    property string userDid
    property Skywalker skywalker: root.getSkywalker(userDid)
    required property int modelId
    required property detailedprofile author
    readonly property string sideBarTitle: qsTr("Update lists")
    readonly property string sideBarDescription: qsTr(`Add/remove ${author.name}`)
    readonly property SvgImage sideBarButtonSvg: SvgOutline.add
    readonly property string sideBarButtonName: qsTr("add new list")

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
        height: portraitHeader.visible ? portraitHeader.height : 0
        z: guiSettings.headerZLevel

        SimpleDescriptionHeader {
            id: portraitHeader
            userDid: view.userDid
            title: sideBarTitle
            description: sideBarDescription
            visible: !root.showSideBar
            onClosed: view.closed()

            SvgPlainButton {
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.rightMargin: parent.usedRightMargin + 10
                svg: sideBarButtonSvg
                onClicked: sideBarButtonClicked()
                accessibleName: sideBarButtonName
            }
        }
    }
    headerPositioning: ListView.OverlayHeader

    SkyMenu {
        id: newListMenu
        x: parent.width - width

        CloseMenuItem {
            text: qsTr("<b>Create list</b>")
            Accessible.name: qsTr("close create list menu")
        }
        AccessibleMenuItem {
            text: qsTr("User list")
            svg: SvgOutline.user
            onTriggered: root.newList(view.model, QEnums.LIST_PURPOSE_CURATE, userDid)
        }
        AccessibleMenuItem {
            text: qsTr("Moderation list")
            svg: SvgOutline.moderation
            onTriggered: root.newList(view.model, QEnums.LIST_PURPOSE_MOD, userDid)
        }
    }

    delegate: AddUserListViewDelegate {
        width: view.width

        onAddToList: (listUri) => graphUtils.addListUser(listUri, author)
        onRemoveFromList: (listUri, listItemUri) => graphUtils.removeListUser(listUri, listItemUri)
    }

    FlickableRefresher {
        inProgress: view.model?.getFeedInProgress
        topOvershootFun: () => skywalker.getListList(modelId)
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
        skywalker: view.skywalker

        onAddListUserFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
        onRemoveListUserFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
    }

    function sideBarButtonClicked() {
        newListMenu.open()
    }

    function refresh() {
        skywalker.getListList(modelId)
    }

    Component.onDestruction: {
        skywalker.removeListListModel(modelId)
    }
}
