import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

ListView {
    required property var skywalker
    required property int modelId
    property bool showAsHome: false
    property int unreadPosts: 0

    signal closed

    id: postFeedView
    spacing: 0
    model: skywalker.getPostFeedModel(modelId)
    clip: true
    flickDeceleration: guiSettings.flickDeceleration
    ScrollIndicator.vertical: ScrollIndicator {}

    header: Rectangle {
        width: parent.width
        height: guiSettings.headerHeight
        z: guiSettings.headerZLevel
        color: guiSettings.headerColor

        RowLayout {
            id: headerRow
            width: parent.width
            height: guiSettings.headerHeight

            SvgButton {
                id: backButton
                iconColor: guiSettings.headerTextColor
                Material.background: "transparent"
                svg: svgOutline.arrowBack
                visible: !showAsHome

                onClicked: postFeedView.closed()
            }
            FeedAvatar {
                Layout.leftMargin: backButton.visible ? 0 : 10
                Layout.rightMargin: 10
                height: parent.height - 10
                width: height
                avatarUrl: postFeedView.model.getGeneratorView().avatar
            }
            Text {
                id: headerTexts
                Layout.fillWidth: !showAsHome
                Layout.alignment: Qt.AlignVCenter
                font.bold: true
                font.pointSize: guiSettings.scaledFont(10/8)
                color: guiSettings.headerTextColor
                text: postFeedView.model.feedName

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if (expandFeedsButton.visible)
                            expandFeedsButton.onClicked()
                    }
                }
            }
            ExpandFeedsButton {
                id: expandFeedsButton
                skywalker: postFeedView.skywalker
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignLeft
                visible: showAsHome
            }
            Item {
                Layout.rightMargin: 10
                Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                height: parent.height - 10
                width: height
                visible: showAsHome

                Avatar {
                    id: avatar
                    width: parent.width
                    height: parent.height
                    avatarUrl: skywalker.avatarUrl
                    onClicked: root.showSettingsDrawer()
                    onPressAndHold: root.showSwitchUserDrawer()
                }
            }
        }
    }
    headerPositioning: ListView.OverlayHeader

    footer: SkyFooter {
        visible: showAsHome
        timeline: postFeedView
        skywalker: postFeedView.skywalker
        homeActive: true
        onHomeClicked: postFeedView.positionViewAtBeginning()
        onNotificationsClicked: root.viewNotifications()
        onSearchClicked: root.viewSearchView()
        onFeedsClicked: root.viewFeedsView()
    }
    footerPositioning: ListView.OverlayFooter

    delegate: PostFeedViewDelegate {
        viewWidth: postFeedView.width
    }

    FlickableRefresher {
        inProgress: skywalker.getFeedInProgress
        verticalOvershoot: postFeedView.verticalOvershoot
        topOvershootFun: () => skywalker.getFeed(modelId)
        bottomOvershootFun: () => skywalker.getFeedNextPage(modelId)
        topText: qsTr("Pull down to refresh feed")
    }

    SvgImage {
        id: noPostImage
        width: 150
        height: 150
        y: height + (parent.headerItem ? parent.headerItem.height : 0)
        anchors.horizontalCenter: parent.horizontalCenter
        color: Material.color(Material.Grey)
        svg: svgOutline.noPosts
        visible: postFeedView.count === 0
    }
    Text {
        id: noPostText
        y: noPostImage.y
        anchors.horizontalCenter: parent.horizontalCenter
        font.pointSize: guiSettings.scaledFont(10/8)
        color: Material.color(Material.Grey)
        elide: Text.ElideRight
        text: qsTr("Feed is empty")
        visible: postFeedView.count === 0
    }

    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        running: skywalker.getFeedInProgress
    }

    GuiSettings {
        id: guiSettings
    }

    function forceDestroy() {
        postFeedView.model = null
        skywalker.removePostFeedModel(modelId)
        modelId = -1
        destroy()
    }

    Component.onDestruction: {
        if (modelId !== -1)
            skywalker.removePostFeedModel(modelId)
    }
}
