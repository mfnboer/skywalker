import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

ListView {
    required property string title
    required property var skywalker
    required property int modelId
    property string description
    property bool showFollow: true

    signal closed

    id: authorListView
    spacing: 0
    model: skywalker.getAuthorListModel(modelId)
    flickDeceleration: guiSettings.flickDeceleration
    ScrollIndicator.vertical: ScrollIndicator {}

    header: SimpleDescriptionHeader {
        title: authorListView.title
        description: authorListView.description
        onClosed: authorListView.closed()
    }
    headerPositioning: ListView.OverlayHeader

    delegate: AuthorViewDelegate {
        viewWidth: authorListView.width
        showFollow: authorListView.showFollow
        onFollow: (profile) => { graphUtils.follow(profile) }
        onUnfollow: (did, uri) => { graphUtils.unfollow(did, uri) }
    }

    FlickableRefresher {
        inProgress: skywalker.getAuthorListInProgress
        verticalOvershoot: authorListView.verticalOvershoot
        bottomOvershootFun: () => skywalker.getAuthorListNextPage(modelId)
        topText: ""
    }

    SvgImage {
        id: noAuthorsImage
        width: 150
        height: 150
        y: height + (parent.headerItem ? parent.headerItem.height : 0)
        anchors.horizontalCenter: parent.horizontalCenter
        color: Material.color(Material.Grey)
        svg: svgOutline.noUsers
        visible: authorListView.count === 0
    }
    Text {
        id: noPostText
        y: noAuthorsImage.y
        anchors.horizontalCenter: parent.horizontalCenter
        font.pointSize: guiSettings.scaledFont(10/8)
        color: Material.color(Material.Grey)
        elide: Text.ElideRight
        text: qsTr("None")
        visible: authorListView.count === 0
    }

    GraphUtils {
        id: graphUtils
        skywalker: authorListView.skywalker

        onFollowFailed: (error) => { statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR) }
        onUnfollowFailed: (error) => { statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR) }
    }

    BusyIndicator {
        id: busyBottomIndicator
        anchors.centerIn: parent
        running: skywalker.getAuthorListInProgress
    }

    GuiSettings {
        id: guiSettings
    }

    Component.onDestruction: {
        skywalker.removeAuthorListModel(modelId)
    }
}
