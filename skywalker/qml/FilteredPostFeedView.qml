import QtQuick
import QtQuick.Controls
import skywalker

SkyListView {
    property string userDid
    property Skywalker skywalker: root.getSkywalker(userDid)
    required property int modelId
    readonly property string sideBarTitle: model.feedName
    readonly property SvgImage sideBarSvg: SvgOutline.hideVisibility

    signal closed

    id: postFeedView
    model: skywalker.getPostFeedModel(modelId)
    boundsBehavior: Flickable.StopAtBounds

    Accessible.name: sideBarTitle

    header: PostFeedHeader {
        userDid: postFeedView.userDid
        feedName: sideBarTitle
        defaultSvg: SvgOutline.hideVisibility
        visible: !root.showSideBar

        onClosed: postFeedView.closed()
    }
    headerPositioning: ListView.OverlayHeader

    footer: DeadFooterMargin {}
    footerPositioning: ListView.OverlayFooter

    delegate: PostFeedViewDelegate {
        width: postFeedView.width
    }

    FlickableRefresher {
        enableScrollToTop: true
    }

    EmptyListIndication {
        y: parent.headerItem ? parent.headerItem.height : 0
        svg: SvgOutline.hideVisibility
        text: qsTr("Feed is empty")
        list: postFeedView
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
