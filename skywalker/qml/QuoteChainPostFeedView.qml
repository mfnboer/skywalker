import QtQuick
import QtQuick.Controls
import skywalker

SkyListView {
    property string userDid
    property Skywalker skywalker: root.getSkywalker(userDid)
    required property int modelId
    readonly property string sideBarTitle: model.feedName
    readonly property SvgImage sideBarSvg: SvgOutline.quote

    signal closed

    id: postFeedView
    model: skywalker.getPostFeedModel(modelId)

    Accessible.name: sideBarTitle

    header: PostFeedHeader {
        userDid: postFeedView.userDid
        feedName: sideBarTitle
        defaultSvg: SvgFilled.quote
        visible: !root.showSideBar

        onClosed: postFeedView.closed()
    }
    headerPositioning: ListView.OverlayHeader

    delegate: PostFeedViewDelegate {
        width: postFeedView.width
        showRecord: false

        // Show SVG at top and bottom. They exactly overlap.
        // However half of the SVG is covered by the next or previous delegate.
        Rectangle {
            anchors.top: parent.top
            anchors.topMargin: -height / 2
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.horizontalCenterOffset: guiSettings.threadColumnWidth / 2
            width: 30
            height: width
            color: guiSettings.backgroundColor
            visible: index > 0

            SkySvg {
                width: parent.width
                height: width
                svg: SvgFilled.quote
            }
        }

        Rectangle {
            anchors.top: parent.bottom
            anchors.topMargin: -height / 2
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.horizontalCenterOffset: guiSettings.threadColumnWidth / 2
            width: 30
            height: width
            color: guiSettings.backgroundColor
            visible: !endOfFeed

            SkySvg {
                width: parent.width
                height: width
                svg: SvgFilled.quote
            }
        }
    }

    onMovementEnded: {
        const lastVisibleIndex = getLastVisibleIndex()

        if (count - lastVisibleIndex < skywalker.QUOTE_CHAIN_PAGE_SIZE - 2 && !model.getFeedInProgress) {
            console.debug("Get next feed page")
            skywalker.getQuoteChainNextPage(modelId)
        }
    }

    onErrorChanged: {
        if (error)
            skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
    }

    FlickableRefresher {
        inProgress: model.getFeedInProgress
        verticalOvershoot: postFeedView.verticalOvershoot
        bottomOvershootFun: () => skywalker.getQuoteChainNextPage(modelId)
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
