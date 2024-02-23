import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

ListView {
    required property var skywalker

    signal closed

    id: view
    spacing: 0
    model: skywalker.createDraftPostsModel()
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
        viewWidth: view.width
    }

    EmptyListIndication {
        y: parent.headerItem ? parent.headerItem.height : 0
        svg: svgOutline.noPosts
        text: qsTr("No drafts")
        list: view
    }

    DraftPosts {
        id: draftPosts
    }

    GuiSettings {
        id: guiSettings
    }

    Component.onDestruction: {
        skywalker.deleteDraftPostsModel()
    }

    Component.onCompleted: {
        draftPosts.loadDraftPostsModel(model)
    }
}
