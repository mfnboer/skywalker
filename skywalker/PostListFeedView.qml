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

    id: postListFeedView
    width: parent.width
    model: skywalker.getPostFeedModel(modelId)

    Accessible.name: postListFeedView.model.getUnderlyingModel().feedName

    header: PostFeedHeader {
        skywalker: postListFeedView.skywalker
        feedName: postListFeedView.model.getUnderlyingModel().feedName
        defaultSvg: SvgFilled.list
        feedAvatar: postListFeedView.model.getUnderlyingModel().getListView().avatarThumb
        showAsHome: postListFeedView.showAsHome
        showLanguageFilter: postListFeedView.model.getUnderlyingModel().languageFilterConfigured
        filteredLanguages: postListFeedView.model.getUnderlyingModel().filteredLanguages
        showPostWithMissingLanguage: postListFeedView.model.getUnderlyingModel().showPostWithMissingLanguage
        showViewOptions: true

        onClosed: postListFeedView.closed()

        onFeedAvatarClicked: {
            let list = postListFeedView.model.getUnderlyingModel().getListView()
            root.viewListByUri(list.uri, false)
        }

        onContentModeChanged: changeView(contentMode)
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
        onMessagesClicked: root.viewChat()
    }
    footerPositioning: ListView.OverlayFooter

    delegate: PostFeedViewDelegate {
        required property int index

        width: postListFeedView.width
        swipeMode: model.contentMode === QEnums.CONTENT_MODE_VIDEO

        onActivateSwipe: {
            root.viewVideoFeed(model, index, (newIndex) => { postListFeedView.positionViewAtIndex(newIndex, ListView.Beginning) })
        }
    }

    FlickableRefresher {
        inProgress: skywalker.getFeedInProgress
        topOvershootFun: () => model.getFeed(skywalker)
        bottomOvershootFun: () => model.getFeedNextPage(skywalker)
        topText: qsTr("Pull down to refresh feed")
        enableScrollToTop: !showAsHome
    }

    EmptyListIndication {
        y: parent.headerItem ? parent.headerItem.height : 0
        svg: SvgOutline.noPosts
        text: qsTr("Feed is empty")
        list: postListFeedView
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
