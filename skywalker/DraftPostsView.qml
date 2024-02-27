import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

ListView {
    signal closed
    signal selected(int index)

    id: view
    spacing: 0
    clip: true
    flickDeceleration: guiSettings.flickDeceleration
    boundsBehavior: Flickable.StopAtBounds
    ScrollIndicator.vertical: ScrollIndicator {}

    Accessible.role: Accessible.List

    header: SimpleHeader {
        text: qsTr("Drafts")
        onBack: view.closed()
    }
    headerPositioning: ListView.OverlayHeader

    delegate: DraftPostViewDelegate {
        required property int index
        viewWidth: view.width
        onSelected: view.selected(index)
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
