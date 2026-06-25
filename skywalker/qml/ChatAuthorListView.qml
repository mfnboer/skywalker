import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyListView {
    property string userDid
    required property convoview convo
    property Skywalker skywalker: root.getSkywalker(userDid)
    property bool showFollow: true
    property bool showActivitySubscription: false
    readonly property bool userIsOwner: convo.getMember(userDid).groupMember.role === QEnums.CONVO_MEMBER_ROLE_OWNER
    property bool allowDeleteItem: userIsOwner
    readonly property string sideBarTitle: convo.group.name
    readonly property string sideBarSubTitle: qsTr(`Members (${convo.group.memberCount}/${convo.group.memberLimit})`)
    readonly property SvgImage sideBarSvg: SvgOutline.group

    signal closed

    id: authorListView
    width: parent.width
    height: parent.height
    model: skywalker.chat.getConvoMemberListModel(convo.id)

    Accessible.role: Accessible.List

    header: Item {
        width: parent.width
        height: portraitHeader.height
        z: guiSettings.headerZLevel

        SimpleHeader {
            id: portraitHeader
            userDid: authorListView.userDid
            text: sideBarTitle
            subTitle: sideBarSubTitle
            visible: !root.showSideBar
            onBack: authorListView.closed()
        }        
    }
    headerPositioning: ListView.OverlayHeader

    footer: Rectangle {
        width: parent.width
        height: guiSettings.footerHeight
        z: guiSettings.footerZLevel
        color: guiSettings.backgroundColor
        visible: userIsOwner && !convo.group.isLocked()

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
            onClicked: () => authorListView.addMember()
        }
    }
    footerPositioning: ListView.OverlayFooter

    delegate: ChatAuthorViewDelegate {
        required property int index

        width: authorListView.width
        userDid: authorListView.userDid
        showFollow: authorListView.showFollow
        allowDeleteItem: authorListView.allowDeleteItem

        onFollow: (profile) => { graphUtils.follow(profile) }
        onUnfollow: (did, uri) => { graphUtils.unfollow(did, uri) }
        onDeleteItem: (did) => authorListView.removeMember(did)
    }

    onErrorChanged: {
        if (error)
            skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
    }

    FlickableRefresher {
        inProgress: authorListView.model?.getFeedInProgress
        topOvershootFun: () => skywalker.chat.getConvoMembers(convo.id)
        bottomOvershootFun: () => skywalker.chat.getConvoMembersNextPage(convo.id)
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
    }

    BusyIndicator {
        anchors.centerIn: parent
        running: authorListView.model?.getFeedInProgress
    }

    BusyIndicator {
        anchors.centerIn: parent
        running: skywalker.chat.convoUpdateInProgress
    }

    Connections {
        target: skywalker.chat

        function onConvoUpdated(convo) {
            if (convo.id === authorListView.convo.id)
                authorListView.convo = convo
        }

        function onRemoveMemberFailed(msg) {
            skywalker.showStatusMessage(msg, QEnums.STATUS_LEVEL_ERROR)
        }
    }

    function addMember() {
        let component = guiSettings.createComponent("SearchAuthor.qml")
        let searchPage = component.createObject(authorListView, {
                skywalker: skywalker,
                presetTypeaheadList: skywalker.chat.getAcceptedConvoMembers(),
                searchFilter: QEnums.AUTHOR_SEARCH_FILTER_GROUP_CHAT_ONLY
        })
        searchPage.onAuthorClicked.connect((profile) => { // qmllint disable missing-property
            skywalker.chat.addMember(convo.id, profile.did)
            root.popStack()
        })
        searchPage.onClosed.connect(() => { root.popStack() })
        root.pushStack(searchPage)
    }

    function removeMember(did) {
        skywalker.chat.removeMember(convo.id, did)
    }

    Component.onDestruction: {
        skywalker.chat.removeConvoMemberListModel(convo.id)
    }

    Component.onCompleted: {
        if (count <= 0)
            skywalker.chat.getConvoMembers(convo.id)
    }
}
