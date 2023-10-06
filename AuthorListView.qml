import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

ListView {
    required property string title
    required property var skywalker
    required property int modelId
    property int margin: 8

    property bool inBottomOvershoot: false

    signal closed

    id: authorListView
    spacing: 0
    model: skywalker.getAuthorListModel(modelId)
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
                iconColor: "white"
                Material.background: "transparent"
                svg: svgOutline.arrowBack
                onClicked: authorListView.closed()
            }
            Text {
                id: headerTexts
                Layout.alignment: Qt.AlignVCenter
                Layout.fillWidth: true
                leftPadding: 10
                font.bold: true
                font.pointSize: guiSettings.scaledFont(10/8)
                color: "white"
                text: title
            }
        }
    }
    headerPositioning: ListView.OverlayHeader

    delegate: AuthorViewDelegate {
        viewWidth: authorListView.width
        onFollow: (profile) => { graphUtils.follow(profile) }
        onUnfollow: (did, uri) => { graphUtils.unfollow(did, uri) }
    }

    onVerticalOvershootChanged: {
        if (verticalOvershoot > 0) {
            if (!inBottomOvershoot && !skywalker.getAuthorListInProgress) {
                skywalker.getAuthorListNextPage(modelId)
            }

            inBottomOvershoot = true;
        } else {
            inBottomOvershoot = false;
        }
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
