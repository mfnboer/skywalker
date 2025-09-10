import QtQuick
import QtQuick.Layouts
import skywalker

RowLayout {
    required property var notificationPref

    Layout.fillWidth: true
    spacing: -1

    SkyRadioButton {
        Layout.fillWidth: true
        text: qsTr("In-app + Push")
        checked: notificationPref.list && notificationPref.push
        onCheckedChanged: {
            if (checked) {
                notificationPref.list = true
                notificationPref.push = true
            }
        }
    }
    SkyRadioButton {
        Layout.fillWidth: true
        text: qsTr("In-app")
        checked: notificationPref.list && !notificationPref.push
        onCheckedChanged: {
            if (checked) {
                notificationPref.list = true
                notificationPref.push = false
            }
        }
    }
    SkyRadioButton {
        Layout.fillWidth: true
        text: qsTr("Off")
        checked: !notificationPref.list
        onCheckedChanged: {
            if (checked)
                notificationPref.list = false
        }
    }
}
