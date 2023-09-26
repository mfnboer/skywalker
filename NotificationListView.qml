import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

ListView {

    required property var skywalker
    property int margin: 8

    property bool inTopOvershoot: false
    property bool inBottomOvershoot: false

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

    GuiSettings {
        id: guiSettings
    }
}
