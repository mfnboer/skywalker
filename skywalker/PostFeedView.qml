import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyListView {
    required property var skywalker
    required property int modelId
    property bool showAsHome: false
    property int unreadPosts: 0

    signal closed

    id: postFeedView
    width: parent.width
    model: skywalker.getPostFeedModel(modelId)

    Accessible.name: postFeedView.model.feedName

    header: PostFeedHeader {
        skywalker: postFeedView.skywalker
        feedName: postFeedView.model.feedName
        feedAvatar: guiSettings.contentVisible(postFeedView.model.getGeneratorView()) ? postFeedView.model.getGeneratorView().avatarThumb : ""
        showAsHome: postFeedView.showAsHome
        showLanguageFilter: postFeedView.model.languageFilterConfigured
        filteredLanguages: postFeedView.model.filteredLanguages
        showPostWithMissingLanguage: postFeedView.model.showPostWithMissingLanguage

        onClosed: postFeedView.closed()
        onFeedAvatarClicked: skywalker.getFeedGenerator(postFeedView.model.getGeneratorView().uri)
    }
    headerPositioning: ListView.OverlayHeader

    footer: SkyFooter {
        visible: showAsHome
        timeline: postFeedView
        skywalker: postFeedView.skywalker
        homeActive: true
        showHomeFeedBadge: true
        onHomeClicked: postFeedView.positionViewAtBeginning()
        onNotificationsClicked: root.viewNotifications()
        onSearchClicked: root.viewSearchView()
        onFeedsClicked: root.viewFeedsView()
        onMessagesClicked: root.viewChat()
    }
    footerPositioning: ListView.OverlayFooter

    delegate: PostFeedViewDelegate {
        width: postFeedView.width
    }

    FlickableRefresher {
        inProgress: skywalker.getFeedInProgress
        verticalOvershoot: postFeedView.verticalOvershoot
        topOvershootFun: () => skywalker.getFeed(modelId)
        bottomOvershootFun: () => skywalker.getFeedNextPage(modelId)
        topText: qsTr("Pull down to refresh feed")
        enableScrollToTop: !showAsHome
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
        running: skywalker.getFeedInProgress
    }


    function forceDestroy() {
        if (modelId !== -1) {
            postFeedView.model = null
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
