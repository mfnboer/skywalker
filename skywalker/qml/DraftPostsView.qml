import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyListView {
    required property DraftPosts draftPosts

    signal selected(int index)
    signal deleted(int index)

    id: view
    model: draftPosts.getDraftPostsModel()
    boundsBehavior: Flickable.StopAtBounds

    delegate: DraftPostViewDelegate {
        required property int index

        width: view.width
        onSelected: view.selected(index)
        onDeleted: view.deleted(index)
    }

    FlickableRefresher {
        inProgress: model.getFeedInProgress
        topOvershootFun: () => draftPosts.loadDraftPosts()
        bottomOvershootFun: () => draftPosts.loadDraftPostsNextPage()
        topText: qsTr("Pull down to refresh drafts")
    }

    EmptyListIndication {
        y: parent.headerItem ? parent.headerItem.height : 0
        svg: SvgOutline.noPosts
        text: qsTr("No drafts")
        list: view
    }

    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        running: model.getFeedInProgress
    }
}
