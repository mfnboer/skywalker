import QtQuick
import QtQuick.Controls
import skywalker

SkyListView {
    property string userDid
    property Skywalker skywalker: root.getSkywalker(userDid)
    required property int modelId
    readonly property string sideBarTitle: model.feedName
    readonly property SvgImage sideBarSvg: SvgOutline.repost

    signal closed

    id: postFeedView
    model: skywalker.getPostFeedModel(modelId)

    Accessible.name: sideBarTitle

    header: PostFeedHeader {
        userDid: postFeedView.userDid
        feedName: sideBarTitle
        defaultSvg: SvgFilled.repost
        visible: !root.showSideBar

        onClosed: postFeedView.closed()
    }
    headerPositioning: ListView.OverlayHeader

    delegate: PostFeedViewDelegate {
        width: postFeedView.width
    }

    FlickableRefresher {
        inProgress: model.getFeedInProgress
        verticalOvershoot: postFeedView.verticalOvershoot
        bottomOvershootFun: () => skywalker.getQuotesFeedNextPage(modelId)
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
