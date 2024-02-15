import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

ListView {
    required property var skywalker
    required property int modelId
    property bool showAsHome: false
    property int unreadPosts: 0

    signal closed

    id: postListFeedView
    spacing: 0
    model: skywalker.getPostFeedModel(modelId)
    clip: true
    flickDeceleration: guiSettings.flickDeceleration
    ScrollIndicator.vertical: ScrollIndicator {}

    Accessible.role: Accessible.List
    Accessible.name: postListFeedView.model.feedName

    header: PostFeedHeader {
        skywalker: postListFeedView.skywalker
        feedName: postListFeedView.model.feedName
        isList: true
        feedAvatar: postListFeedView.model.getListView().avatar
        showAsHome: postListFeedView.showAsHome

        onClosed: postListFeedView.closed()
        onFeedAvatarClicked: {
            let list = postListFeedView.model.getListView()
            root.viewListByUri(list.uri, false)
        }
    }
    headerPositioning: ListView.OverlayHeader

    footer: SkyFooter {
        visible: showAsHome
        timeline: postListFeedView
        skywalker: postListFeedView.skywalker
        homeActive: true
        showHomeFeedBadge: true
        onHomeClicked: postListFeedView.positionViewAtBeginning()
        onNotificationsClicked: root.viewNotifications()
        onSearchClicked: root.viewSearchView()
        onFeedsClicked: root.viewFeedsView()
    }
    footerPositioning: ListView.OverlayFooter

    delegate: PostFeedViewDelegate {
        viewWidth: postListFeedView.width
    }

    FlickableRefresher {
        inProgress: skywalker.getFeedInProgress
        verticalOvershoot: postListFeedView.verticalOvershoot
        topOvershootFun: () => skywalker.getListFeed(modelId)
        bottomOvershootFun: () => skywalker.getListFeedNextPage(modelId)
        topText: qsTr("Pull down to refresh feed")
    }

    EmptyListIndication {
        y: parent.headerItem ? parent.headerItem.height : 0
        svg: svgOutline.noPosts
        text: qsTr("Feed is empty")
        list: postListFeedView
    }

    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        running: skywalker.getFeedInProgress
    }

    GuiSettings {
        id: guiSettings
    }

    function forceDestroy() {
        if (modelId !== -1) {
            postListFeedView.model = null
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
