import QtQuick
import QtQuick.Layouts

ColumnLayout {
    property var skywalker: root.getSkywalker()
    property var userSettings: skywalker.getUserSettings()

    HeaderText {
        Layout.topMargin: 10
        text: qsTr("Notifications")
    }

    GridLayout {
        Layout.fillWidth: true
        columns: 2
        rowSpacing: 5

        AccessibleText {
            Layout.preferredWidth: 120
            text: qsTr("Likes")
        }

        NotificationTypeSetting {}

        AccessibleText {
            Layout.preferredWidth: 120
            text: qsTr("Likes from")
        }

        NotificationIncludeSetting {}
    }

    AccessibleText {
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        text: qsTr("Push notifications can be enabled/disabled in the app settings of your phone.")
    }

    AccessibleCheckBox {
        text: qsTr("WiFi only")
        checked: userSettings.getNotificationsWifiOnly()
        onCheckedChanged: userSettings.setNotificationsWifiOnly(checked)
    }
}
