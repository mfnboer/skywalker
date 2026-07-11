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
    property bool showDescription: true
    property bool allowDeleteItem: false
    property bool allowAddItem: false
    property int maxItems: 0
    property string listUri // set when the author list is a list of members from listUri
    property int prevModelId: -1
    readonly property string sideBarTitle: authorListView.title
    readonly property string sideBarDescription: authorListView.description
    readonly property SvgImage sideBarSvg: SvgOutline.group
    property var sideBarButtonSvg: undefined
    property string sideBarButtonName

    signal closed
    signal sideBarButtonClicked(Item item, point p)

    id: authorListView
    model: skywalker.getAuthorListModel(modelId)

    Accessible.role: Accessible.List

    header: Item {
        width: parent.width
        height: portraitHeader.height
        z: guiSettings.headerZLevel

        SimpleDescriptionHeader {
            id: portraitHeader
            userDid: authorListView.userDid
            title: sideBarTitle
            subTitle: maxItems > 0 ? `${count} / ${maxItems}` : ""
            description: sideBarDescription
            visible: authorListView.title && !root.showSideBar
            onClosed: authorListView.closed()

            Loader {
                anchors.right: parent.right
                anchors.top: parent.top
                active: sideBarButtonSvg != undefined

                sourceComponent: SvgPlainButton {
                    svg: sideBarButtonSvg
                    accessibleName: sideBarButtonName
                    onClicked: sideBarButtonClicked(authorListView.header, Qt.point(x, y))
                }
            }
        }
    }
    headerPositioning: ListView.OverlayHeader

    footer: Rectangle {
        width: parent.width
        height: guiSettings.footerHeight
        z: guiSettings.footerZLevel
        color: guiSettings.backgroundColor
        enabled: maxItems <= 0 || count < maxItems
        visible: allowAddItem

        SvgButton {
            anchors.horizontalCenter: parent.horizontalCenter
            width: height
            height: parent.height
            topInset: 0
            leftInset: 0
            rightInset: 0
            bottomInset: 0
            svg: SvgOutline.addUser
            accessibleName: qsTr("add user to group")
            onClicked: () => authorListView.addListItem()
        }
    }
    footerPositioning: ListView.OverlayFooter

    delegate: AuthorViewDelegate {
        required property int index

        width: authorListView.width
        userDid: authorListView.userDid
        showFollow: authorListView.showFollow
        showActivitySubscription: authorListView.showActivitySubscription
        showDescription: authorListView.showDescription
        allowDeleteItem: authorListView.allowDeleteItem

        onFollow: (profile) => { graphUtils.follow(profile) }
        onUnfollow: (did, uri) => { graphUtils.unfollow(did, uri) }
        onDeleteItem: (listItemUri, did) => authorListView.deleteListItem(listItemUri, did, index)
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

        onCreatedTrustedVerifierList: (uri) => model.setAtId(uri)

        onAddTrustedVerifierOk: (profile, itemUri) => model.prependBasicProfile(profile, itemUri)

        onAddTrustedVerifierFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
    }

    BusyIndicator {
        id: busyBottomIndicator
        anchors.centerIn: parent
        running: authorListView.model?.getFeedInProgress
    }

    function addListItem() {
        if (model.type !== QEnums.AUTHOR_LIST_TRUSTED_VERIFIERS) {
            console.warn("Cannot add profile, unexpected model:", model.type)
            return
        }

        let component = guiSettings.createComponent("SearchAuthor.qml")
        let searchPage = component.createObject(authorListView, {
                skywalker: skywalker
        })
        searchPage.onAuthorClicked.connect((profile) => { // qmllint disable missing-property
            graphUtils.addTrustedVerifier(profile)
            root.popStack()
        })
        searchPage.onClosed.connect(() => { root.popStack() })
        root.pushStack(searchPage)
    }

    function deleteListItem(listItemUri, did, index) {
        if (model.type === QEnums.AUTHOR_LIST_TRUSTED_VERIFIERS) {
            model.deleteEntry(index)
            graphUtils.removeTrustedVerifier(did)
        } else {
            model.deleteEntry(index)
            graphUtils.removeListUser(listUri, listItemUri)
        }
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
