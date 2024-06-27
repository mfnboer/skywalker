import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

ListView {
    required property detailedprofile author
    required property var enclosingView
    required property var getFeed
    required property var getFeedNextPage
    required property var getEmptyListIndicationSvg
    required property var getEmptyListIndicationText
    required property var visibilityShowProfileLink
    required property var disableWarning
    property int modelId: -1
    property int feedFilter: QEnums.AUTHOR_FEED_FILTER_POSTS

    id: authorPostsList
    width: parent.width
    height: parent.height
    clip: true
    spacing: 0
    model: modelId >= 0 ? skywalker.getAuthorFeedModel(page.modelId) : null
    flickDeceleration: guiSettings.flickDeceleration
    maximumFlickVelocity: maxFlickVelocity
    pixelAligned: guiSettings.flickPixelAligned
    ScrollIndicator.vertical: ScrollIndicator {}
    interactive: !enclosingView.interactive

    StackLayout.onIsCurrentItemChanged: {
        if (StackLayout.isCurrentItem && modelId < 0 && !skywalker.getAuthorFeedInProgress) {
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
    }

    FlickableRefresher {
        inProgress: skywalker.getAuthorFeedInProgress
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
        running: skywalker.getAuthorFeedInProgress
    }

    EmptyListIndication {
        id: noPostIndication
        svg: getEmptyListIndicationSvg()
        text: getEmptyListIndicationText()
        list: authorPostsList
        onLinkActivated: (link) => root.viewListByUri(link, false)
    }
    Text {
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

    GuiSettings {
        id: guiSettings
    }

    function feedOk() {
        retryGetFeedTimer.resetRetryAttempts()
    }

    function retryGetFeed() {
        return retryGetFeedTimer.retry()
    }

    function refresh() {
        if (modelId >= 0)
            getFeed(modelId)
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
