import QtQuick
import QtQuick.Controls
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

    Accessible.name: postFeedView.model.getUnderlyingModel().feedName

    header: PostFeedHeader {
        skywalker: postFeedView.skywalker
        feedName: postFeedView.model.getUnderlyingModel().feedName
        feedAvatar: guiSettings.contentVisible(postFeedView.model.getUnderlyingModel().getGeneratorView()) ? postFeedView.model.getUnderlyingModel().getGeneratorView().avatarThumb : ""
        defaultSvg: guiSettings.feedDefaultAvatar(postFeedView.model.getUnderlyingModel().getGeneratorView())
        contentMode: postFeedView.model.getUnderlyingModel().contentMode
        showAsHome: postFeedView.showAsHome
        showLanguageFilter: postFeedView.model.getUnderlyingModel().languageFilterConfigured
        filteredLanguages: postFeedView.model.getUnderlyingModel().filteredLanguages
        showPostWithMissingLanguage: postFeedView.model.getUnderlyingModel().showPostWithMissingLanguage
        showViewOptions: true

        onClosed: postFeedView.closed()
        onFeedAvatarClicked: skywalker.getFeedGenerator(postFeedView.model.getUnderlyingModel().getGeneratorView().uri)
        onContentModeChanged: changeView(contentMode)
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
        required property int index

        width: postFeedView.width
        swipeMode: model.contentMode === QEnums.CONTENT_MODE_VIDEO

        onActivateSwipe: {
            root.viewVideoFeed(model, index, (newIndex) => { postFeedView.positionViewAtIndex(newIndex, ListView.Beginning) })
        }
    }

    FlickableRefresher {
        inProgress: skywalker.getFeedInProgress
        verticalOvershoot: postFeedView.verticalOvershoot
        topOvershootFun: () => model.getFeed(skywalker)
        bottomOvershootFun: () => model.getFeedNextPage(skywalker)
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

    function changeView(contentMode) {
        let oldModel = model

        switch (contentMode) {
        case QEnums.CONTENT_MODE_UNSPECIFIED:
            model = model.getUnderlyingModel()
            break
        case QEnums.CONTENT_MODE_VIDEO:
            model = model.getUnderlyingModel().addVideoFilter()
            break
        default:
            console.warn("Unknown content mode:", contentMode)
            return
        }

        if (oldModel.isFilterModel())
            oldModel.getUnderlyingModel().deleteFilteredPostFeedModel(oldModel)
    }

    function activate() {
        for (var i = 0; i < count; ++i) {
            const item = itemAtIndex(i)

            if (item)
                item.activate() // qmllint disable missing-property
        }
    }

    function deactivate() {
        for (var i = 0; i < count; ++i) {
            const item = itemAtIndex(i)

            if (item)
                item.deactivate() // qmllint disable missing-property
        }
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
