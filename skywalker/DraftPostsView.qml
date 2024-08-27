import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyListView {
    signal closed
    signal selected(int index)
    signal deleted(int index)

    id: view
    boundsBehavior: Flickable.StopAtBounds

    header: SimpleHeader {
        text: qsTr(`Drafts ${view.count} / ${view.model.getMaxDrafts()}`)
        onBack: view.closed()
    }
    headerPositioning: ListView.OverlayHeader

    delegate: DraftPostViewDelegate {
        required property int index

        width: view.width
        onSelected: view.selected(index)
        onDeleted: view.deleted(index)
    }

    EmptyListIndication {
        y: parent.headerItem ? parent.headerItem.height : 0
        svg: svgOutline.noPosts
        text: qsTr("No drafts")
        list: view
    }

    GuiSettings {
        id: guiSettings
    }
}
