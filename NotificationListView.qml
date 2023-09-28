import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

ListView {

    required property var skywalker
    property int margin: 8

    property bool inTopOvershoot: false
    property bool gettingNewNotifications: false
    property bool inBottomOvershoot: false
    property bool gettingNextPage: false

    signal closed

    id: notificationListView
    spacing: 0
    model: skywalker.notificationListModel
    ScrollIndicator.vertical: ScrollIndicator {}

    header: Rectangle {
        width: parent.width
        height: guiSettings.headerHeight
        z: guiSettings.headerZLevel
        color: guiSettings.headerColor

        RowLayout
        {
            id: headerRow

            SvgButton {
                id: backButton
                iconColor: "white"
                Material.background: "transparent"
                svg: svgOutline.arrowBack
                onClicked: notificationListView.closed()
            }
            Text {
                id: headerTexts
                Layout.alignment: Qt.AlignVCenter
                font.bold: true
                font.pointSize: guiSettings.scaledFont(10/8)
                color: "white"
                text: qsTr("Notifications")
            }
        }
    }
    headerPositioning: ListView.OverlayHeader

    delegate: NotificationViewDelegate {
        viewWidth: notificationListView.width
    }

    onVerticalOvershootChanged: {
        if (verticalOvershoot < 0)  {
            if (!inTopOvershoot && !skywalker.getNotificationsInProgress) {
                gettingNewNotifications = true
                skywalker.getNotifications(50)
            }

            inTopOvershoot = true
        } else {
            inTopOvershoot = false
        }

        if (verticalOvershoot > 0) {
            if (!inBottomOvershoot && !skywalker.getNotificationsInProgress) {
                gettingNextPage = true
                skywalker.getNotificationsNextPage()
            }

            inBottomOvershoot = true;
        } else {
            inBottomOvershoot = false;
        }
    }

    BusyIndicator {
        id: busyTopIndicator
        y: parent.y + guiSettings.headerHeight
        anchors.horizontalCenter: parent.horizontalCenter
        running: gettingNewNotifications
    }

    BusyIndicator {
        id: busyBottomIndicator
        y: parent.y + parent.height - height - guiSettings.footerHeight
        anchors.horizontalCenter: parent.horizontalCenter
        running: gettingNextPage
    }

    GuiSettings {
        id: guiSettings
    }

    Component.onCompleted: {
        skywalker.onGetNotificationsInProgressChanged.connect(() => {
            if (!skywalker.getNotificationsInProgress) {
                gettingNewNotifications = false
                gettingNextPage = false
            }
        })
    }
}
