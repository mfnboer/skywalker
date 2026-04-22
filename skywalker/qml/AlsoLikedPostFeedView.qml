import QtQuick
import QtQuick.Controls
import skywalker

SkyListView {
    property string userDid
    property Skywalker skywalker: root.getSkywalker(userDid)
    required property int modelId
    readonly property string sideBarTitle: model.feedName
    readonly property string sideBarSubTitle: qsTr("Powered by For You")
    readonly property string sideBarFeedAvatarUrl: forYou.avatar
    readonly property string sideBarFeedUri: forYou.uri

    signal closed

    id: postFeedView
    cacheBuffer: Screen.height * 3
    model: skywalker.getPostFeedModel(modelId)

    Accessible.name: sideBarTitle

    header: PostFeedHeader {
        userDid: postFeedView.userDid
        feedName: sideBarTitle
        subTitle: sideBarSubTitle
        feedAvatar: sideBarFeedAvatarUrl
        defaultSvg: SvgFilled.like
        visible: !root.showSideBar

        onClosed: postFeedView.closed()
        onFeedAvatarClicked: skywalker.getFeedGenerator(sideBarFeedUri)
    }
    headerPositioning: ListView.OverlayHeader

    delegate: PostFeedViewDelegate {
        id: postDelegate
        width: postFeedView.width
        extraFooterHeight: index == 0 ? alsoLikedFooter.height : 0
        showAlsoLiked: index > 0

        Column {
            id: alsoLikedFooter
            anchors.bottom: parent.bottom
            width: parent.width
            visible: postDelegate.index == 0

            AccessibleText {
                id: alsoLikedText
                leftPadding: guiSettings.threadColumnWidth
                topPadding: 10
                bottomPadding: 10
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                text: qsTr("People who liked ☝️ also liked 👇")
            }

            Rectangle {
                width: parent.width
                height: 1
                color: guiSettings.separatorColor
            }
        }
    }

    onMovementEnded: {
        const firstVisibleIndex = getFirstVisibleIndex()
        const lastVisibleIndex = getLastVisibleIndex()
        const remaining = count - lastVisibleIndex

        if (remaining < 10 && !model.getFeedInProgress)
            skywalker.getAlsoLikedFeedNextPage(modelId)
    }

    onErrorChanged: {
        if (error)
            skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
    }

    FlickableRefresher {
        inProgress: model.getFeedInProgress
        verticalOvershoot: postFeedView.verticalOvershoot
        bottomOvershootFun: () => skywalker.getAlsoLikedFeedNextPage(modelId)
        enableScrollToTop: true
    }

    EmptyListIndication {
        y: parent.headerItem ? parent.headerItem.height : 0
        svg: SvgOutline.noPosts
        text: qsTr("Feed is empty")
        list: postFeedView
    }

    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        running: model.getFeedInProgress
    }

    ForYou {
        id: forYou
        skywalker: postFeedView.skywalker
    }

    function forceDestroy() {
        if (modelId !== -1) {
            skywalker.removePostFeedModel(modelId)
            modelId = -1
            destroy()
        }
    }

    Component.onDestruction: {
        if (modelId !== -1)
            skywalker.removePostFeedModel(modelId)
    }
}
