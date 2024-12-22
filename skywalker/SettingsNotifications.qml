import QtQuick
import QtQuick.Layouts

ColumnLayout {
    property var skywalker: root.getSkywalker()
    property var userSettings: skywalker.getUserSettings()

    HeaderText {
        Layout.topMargin: 10
        text: qsTr("Pull notifications")
    }

    AccessibleText {
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        text: qsTr("Notifications can be enabled/disabled in the app settings of your phone.")
    }

    AccessibleSwitch {
        text: qsTr("WiFi only")
        checked: userSettings.getNotificationsWifiOnly()
        onCheckedChanged: userSettings.setNotificationsWifiOnly(checked)
    }
}
