import QtQuick
import QtQuick.Layouts
import skywalker

RowLayout {
    required property var filterablePref // EditChatNotificationPref or EditNotificationFilterablePref

    Layout.fillWidth: true
    spacing: -1

    SkyRadioButton {
        Layout.fillWidth: true
        text: qsTr("Everyone")
        checked: filterablePref.includeType === QEnums.NOTIFICATION_FILTER_INCLUDE_ALL
        enabled: filterablePref instanceof EditChatNotificationPref ? filterablePref.push : filterablePref.list
        onCheckedChanged: {
            if (checked)
                filterablePref.includeType = QEnums.NOTIFICATION_FILTER_INCLUDE_ALL
        }
    }
    SkyRadioButton {
        Layout.fillWidth: true
        text: qsTr("Users I follow")
        checked: filterablePref.includeType === QEnums.NOTIFICATION_FILTER_INCLUDE_FOLLOWS
        enabled: filterablePref instanceof EditChatNotificationPref ? filterablePref.push : filterablePref.list
        onCheckedChanged: {
            if (checked)
                filterablePref.includeType = QEnums.NOTIFICATION_FILTER_INCLUDE_FOLLOWS
        }
    }
}
