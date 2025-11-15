import QtQuick
import QtQuick.Controls
import skywalker

SkyListView {
    property string userDid
    property Skywalker skywalker: root.getSkywalker(userDid)
    required property int modelId
    readonly property int hideReason: model.getHideReason()
    readonly property string sideBarTitle: model.feedName
    readonly property string sideBarSubTitle: qEnums.hideReasonToString(hideReason)
    readonly property SvgImage sideBarSvg: SvgOutline.hideVisibility

    signal closed

    id: postFeedView
    model: skywalker.getPostFeedModel(modelId)
    boundsBehavior: Flickable.StopAtBounds

    Accessible.name: sideBarTitle

    header: SimpleHeader {
        text: sideBarTitle
        subTitle: sideBarSubTitle
        visible: !root.showSideBar
        onBack: postFeedView.closed()
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
        text: qsTr("No filtered posts")
        list: postFeedView
    }

    QEnums {
        id: qEnums
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
