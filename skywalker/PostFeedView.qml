import QtQuick
import QtQuick.Controls
import skywalker

SkyListView {
    required property var skywalker
    required property int modelId
    property bool showAsHome: false
    property int unreadPosts: 0
    readonly property var underlyingModel: model ? model.getUnderlyingModel() : null

    signal closed

    id: postFeedView
    width: parent.width
    model: skywalker.getPostFeedModel(modelId)

    Accessible.name: underlyingModel ? underlyingModel.feedName : ""

    header: PostFeedHeader {
        skywalker: postFeedView.skywalker
        feedName: underlyingModel ? underlyingModel.feedName : ""
        feedAvatar: getFeedAvatar()
        defaultSvg: getFeedDefaultAvatar()
        contentMode: underlyingModel ? underlyingModel.contentMode : QEnums.CONTENT_MODE_UNSPECIFIED
        showAsHome: postFeedView.showAsHome
        showLanguageFilter: underlyingModel ? underlyingModel.languageFilterConfigured : false
        filteredLanguages: underlyingModel ? underlyingModel.filteredLanguages : []
        showPostWithMissingLanguage: underlyingModel ? underlyingModel.showPostWithMissingLanguage :true
        showViewOptions: true

        onClosed: postFeedView.closed()
        onFeedAvatarClicked: showFeed()
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
        extraFooterHeight: extraFooterLoader.active ? extraFooterLoader.height : 0

        onActivateSwipe: {
            root.viewVideoFeed(model, index, (newIndex) => { postFeedView.positionViewAtIndex(newIndex, ListView.Beginning) })
        }

        Loader {
            id: extraFooterLoader
            anchors.bottom: parent.bottom

            active: model.isFilterModel() && index == count - 1 && !endOfFeed
            sourceComponent: extraFooterComponent
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
        id: emptyListIndication
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

    Component {
        id: extraFooterComponent

        Rectangle {
            width: postFeedView.width
            height: 150
            color: "transparent"

            AccessibleText {
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                padding: 10
                textFormat: Text.RichText
                wrapMode: Text.Wrap
                text: qsTr(`${guiSettings.getFilteredPostsFooterText(model)}<br><a href="load" style="color: ${guiSettings.linkColor}; text-decoration: none">Load more</a>`)
                onLinkActivated: model.getFeedNextPage(skywalker)
            }
        }
    }

    Loader {
        anchors.top: emptyListIndication.bottom
        active: model.isFilterModel() && count === 0 && !model.endOfFeed
        sourceComponent: extraFooterComponent
    }

    function getFeedDefaultAvatar() {
        if (!underlyingModel)
            return SvgFilled.feed

        switch (underlyingModel.feedType) {
        case QEnums.FEED_GENERATOR:
            return guiSettings.feedDefaultAvatar(underlyingModel.getGeneratorView())
        case QEnums.FEED_LIST:
            return SvgFilled.list
        default:
            console.warn("Unexpected feed type:", underlyingModel.feedType)
            return SvgFilled.feed
        }
    }

    function getFeedAvatar() {
        if (!underlyingModel)
            return ""

        switch (underlyingModel.feedType) {
        case QEnums.FEED_GENERATOR:
            return guiSettings.feedContentVisible(underlyingModel.getGeneratorView()) ?
                underlyingModel.getGeneratorView().avatarThumb : ""
        case QEnums.FEED_LIST:
            return guiSettings.feedContentVisible(underlyingModel.getListView()) ?
                underlyingModel.getListView().avatarThumb : ""
        default:
            console.warn("Unexpected feed type:", underlyingModel.feedType)
            return ""
        }
    }

    function showFeed() {
        if (!underlyingModel)
            return

        switch (underlyingModel.feedType) {
        case QEnums.FEED_GENERATOR:
            skywalker.getFeedGenerator(underlyingModel.getGeneratorView().uri)
            break
        case QEnums.FEED_LIST:
            root.viewListByUri(underlyingModel.getListView().uri, false)
            break
        default:
            console.warn("Unexpected feed type:", underlyingModel.feedType)
            break
        }
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
