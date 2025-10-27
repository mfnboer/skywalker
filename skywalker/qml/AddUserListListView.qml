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
            userDid: view.userDid
            title: sideBarTitle
            description: sideBarDescription
            visible: !root.showSideBar
            onClosed: view.closed()

            SvgPlainButton {
                anchors.top: parent.top
                anchors.topMargin: guiSettings.headerMargin
                anchors.right: parent.right
                anchors.rightMargin: parent.usedRightMargin + 10
                svg: SvgOutline.add
                onClicked: newListMenu.open()
                accessibleName: qsTr(`add new list`)

                SkyMenu {
                    id: newListMenu
                    onAboutToShow: root.enablePopupShield(true)
                    onAboutToHide: root.enablePopupShield(false)

                    CloseMenuItem {
                        text: qsTr("<b>Create list</b>")
                        Accessible.name: qsTr("close create list menu")
                    }
                    AccessibleMenuItem {
                        text: qsTr("User list")
                        onTriggered: root.newList(view.model, QEnums.LIST_PURPOSE_CURATE, userDid)
                        MenuItemSvg { svg: SvgOutline.user }
                    }
                    AccessibleMenuItem {
                        text: qsTr("Moderation list")
                        onTriggered: root.newList(view.model, QEnums.LIST_PURPOSE_MOD, userDid)
                        MenuItemSvg { svg: SvgOutline.moderation }
                    }
                }
            }
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

    function refresh() {
        skywalker.getListList(modelId)
    }

    Component.onDestruction: {
        skywalker.removeListListModel(modelId)
    }
}
