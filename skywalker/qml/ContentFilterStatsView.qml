import QtQuick
import QtQuick.Controls
import skywalker

SkyPage {
    required property ContentFilterStatsModel model

    id: page

    signal closed

    header: SimpleHeader {
        text: qsTr("Filter statistics")
        onBack: page.closed()
    }

    footer: DeadFooterMargin {
    }

    TreeView {
        id: treeView
        anchors.fill: parent
        anchors.margins: 10
        clip: true
        model: page.model

        delegate: TreeViewDelegate {
            width: parent.width
        }
    }

    Component.onDestruction: {
        model.destroy()
    }
}
