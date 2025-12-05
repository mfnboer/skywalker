import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyListView {
    property string userDid
    required property detailedprofile author
    required property var enclosingView
    required property var getFeed
    required property var getFeedNextPage
    required property var getEmptyListIndicationSvg
    required property var getEmptyListIndicationText
    required property var getEmptyListIndicationLabeler
    required property var visibilityShowProfileLink
    required property var disableWarning
    property var skywalker: root.getSkywalker(userDid)
    property int modelId: -1
    property int feedFilter: QEnums.AUTHOR_FEED_FILTER_POSTS
    property bool galleryMode: false

    id: authorPostsList
    model: modelId >= 0 ? skywalker.getAuthorFeedModel(page.modelId) : null
    interactive: !enclosingView.interactive
    clip: true

    SwipeView.onIsCurrentItemChanged: changeCurrentItem(SwipeView.isCurrentItem)

    function changeCurrentItem(isCurrentItem) {
        if (!isCurrentItem)
            cover()

        if (isCurrentItem && modelId < 0) {
            modelId = skywalker.createAuthorFeedModel(author, feedFilter)
            model = skywalker.getAuthorFeedModel(modelId)
            getFeed(modelId)
        }
    }

    onVerticalOvershootChanged: {
        if (verticalOvershoot < 0)
            enclosingView.interactive = true
    }

    delegate: PostFeedViewDelegate {
        width: enclosingView.width
        swipeMode: [QEnums.AUTHOR_FEED_FILTER_VIDEO, QEnums.AUTHOR_FEED_FILTER_MEDIA].includes(feedFilter)

        onActivateSwipe: (imgIndex, previewImg) => {
            if (swipeMode)
                root.viewMediaFeed(model, index, imgIndex, previewImg, (newIndex) => { authorPostsList.positionViewAtIndex(newIndex, ListView.Beginning) }, userDid)
            else
                console.warn("This is not a media feed")
        }
    }

    onMovementEnded: updateOnMovement()
    onContentMoved: updateOnMovement()

    function updateOnMovement() {
        if (modelId < 0)
            return

        const lastVisibleIndex = getLastVisibleIndex()

        if (count - lastVisibleIndex < 10 && Boolean(model) && !model.getFeedInProgress) {
            console.debug("Get next author feed page")
            getFeedNextPage(modelId)
        }
    }

    FlickableRefresher {
        inProgress: model && model.getFeedInProgress
        topOvershootFun: () => {
            if (modelId >= 0)
                getFeed(modelId)
        }
        bottomOvershootFun: () => {
            if (modelId >= 0)
                getFeedNextPage(modelId)
        }
        topText: qsTr("Pull down to refresh")
    }

    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        running: model && model.getFeedInProgress
    }

    EmptyListIndication {
        id: noPostIndication
        svg: getEmptyListIndicationSvg()
        text: getEmptyListIndicationText()
        labeler: getEmptyListIndicationLabeler()
        list: authorPostsList
        onLinkActivated: (link) => root.viewListByUri(link, false)
        onRetry: retryGetFeed()
    }
    AccessibleText {
        anchors.top: noPostIndication.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        elide: Text.ElideRight
        textFormat: Text.RichText
        text: `<br><a href=\"show\" style=\"color: ${guiSettings.linkColor}\">` + qsTr("Show profile") + "</a>"
        visible: visibilityShowProfileLink(authorPostsList)
        onLinkActivated: disableWarning()
    }

    Timer {
        readonly property int maxRetry: 1
        property int retryAttempts: maxRetry

        id: retryGetFeedTimer
        interval: 500
        onTriggered: {
            if (modelId >= 0) {
                console.debug("RETRY GET FEED:", modelId)
                getFeed(modelId)
            }
            else {
                console.debug("NO MODEL")
                resetRetryAttempts()
            }
        }

        function retry() {
            if (retryAttempts <= 0) {
                resetRetryAttempts()
                return false
            }

            --retryAttempts
            start()
            return true
        }

        function resetRetryAttempts() {
            console.debug("RESET RETRY ATTEMPTS")
            retryAttempts = maxRetry
        }
    }

    Loader {
        id: mediaTilesLoader
        active: galleryMode && count > 0

        sourceComponent: MediaTilesFeedView {
            clip: true
            width: authorPostsList.width
            height: authorPostsList.height
            userDid: authorPostsList.userDid
            model: authorPostsList.model
            enclosingView: authorPostsList.enclosingView
        }
    }

    function feedOk() {
        retryGetFeedTimer.resetRetryAttempts()
    }

    function retryGetFeed() {
        return retryGetFeedTimer.retry()
    }

    function refresh() {
        if (modelId >= 0)
            getFeed(modelId) // qmllint disable use-proper-function
    }

    function clear() {
        if (modelId >= 0)
            skywalker.clearAuthorFeed(modelId)
    }

    function removeModel() {
        if (modelId >= 0) {
            const id = modelId
            modelId = -1
            skywalker.removeAuthorFeedModel(id)
        }
    }

    Component.onDestruction: {
        removeModel()
    }
}
