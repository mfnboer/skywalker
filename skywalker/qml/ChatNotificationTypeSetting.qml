import QtQuick
import QtQuick.Layouts
import skywalker

RowLayout {
    required property EditChatNotificationPref notificationPref

    Layout.fillWidth: true
    spacing: -1

    SkyRadioButton {
        Layout.fillWidth: true
        leftRadius: guiSettings.radius
        text: qsTr("Push")
        checked: notificationPref.push
        onCheckedChanged: {
            if (checked)
                notificationPref.push = true
        }
    }

    SkyRadioButton {
        Layout.fillWidth: true
        rightRadius: guiSettings.radius
        text: qsTr("Off")
        checked: !notificationPref.push
        onCheckedChanged: {
            if (checked)
                notificationPref.push = false
        }
    }
}
