import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyListView {
    readonly property string sideBarTitle: qsTr("Drafts")
    readonly property string sideBarSubTitle: `${view.count} / ${view.model.getMaxDrafts()}`
    readonly property SvgImage sideBarSvg: SvgOutline.chat

    signal closed
    signal selected(int index)
    signal deleted(int index)

    id: view
    boundsBehavior: Flickable.StopAtBounds

    header: Item {
        width: parent.width
        height: portraitHeader.visible ? portraitHeader.height : landscapeHeader.height
        z: guiSettings.headerZLevel

        SimpleHeader {
            id: portraitHeader
            text: sideBarTitle
            subTitle: sideBarSubTitle
            visible: !root.showSideBar
            onBack: view.closed()
        }
        DeadHeaderMargin {
            id: landscapeHeader
            visible: root.showSideBar
        }
    }
    headerPositioning: ListView.OverlayHeader

    footer: DeadFooterMargin {}
    footerPositioning: ListView.OverlayFooter

    delegate: DraftPostViewDelegate {
        required property int index

        width: view.width
        onSelected: view.selected(index)
        onDeleted: view.deleted(index)
    }

    EmptyListIndication {
        y: parent.headerItem ? parent.headerItem.height : 0
        svg: SvgOutline.noPosts
        text: qsTr("No drafts")
        list: view
    }

}
