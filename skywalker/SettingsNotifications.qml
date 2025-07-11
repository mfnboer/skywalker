import QtQuick
import QtQuick.Layouts

ColumnLayout {
    required property  var notificationPrefs
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
            wrapMode: Text.Wrap
            text: qsTr("Likes")
        }

        NotificationTypeSetting {
            filterablePref: notificationPrefs.like
        }

        AccessibleText {
            Layout.preferredWidth: 120
            wrapMode: Text.Wrap
            text: qsTr("from")
        }

        NotificationIncludeSetting {
            filterablePref: notificationPrefs.like
        }


        AccessibleText {
            Layout.topMargin: 10
            Layout.preferredWidth: 120
            wrapMode: Text.Wrap
            text: qsTr("Likes of reposts")
        }

        NotificationTypeSetting {
            Layout.topMargin: 10
            filterablePref: notificationPrefs.likeViaRepost
        }

        AccessibleText {
            Layout.preferredWidth: 120
            wrapMode: Text.Wrap
            text: qsTr("from")
        }

        NotificationIncludeSetting {
            filterablePref: notificationPrefs.likeViaRepost
        }


        AccessibleText {
            Layout.topMargin: 10
            Layout.preferredWidth: 120
            wrapMode: Text.Wrap
            text: qsTr("New followers")
        }

        NotificationTypeSetting {
            Layout.topMargin: 10
            filterablePref: notificationPrefs.follow
        }

        AccessibleText {
            Layout.preferredWidth: 120
            wrapMode: Text.Wrap
            text: qsTr("from")
        }

        NotificationIncludeSetting {
            filterablePref: notificationPrefs.follow
        }


        AccessibleText {
            Layout.topMargin: 10
            Layout.preferredWidth: 120
            wrapMode: Text.Wrap
            text: qsTr("Replies")
        }

        NotificationTypeSetting {
            Layout.topMargin: 10
            filterablePref: notificationPrefs.reply
        }

        AccessibleText {
            Layout.preferredWidth: 120
            wrapMode: Text.Wrap
            text: qsTr("from")
        }

        NotificationIncludeSetting {
            filterablePref: notificationPrefs.reply
        }


        AccessibleText {
            Layout.topMargin: 10
            Layout.preferredWidth: 120
            wrapMode: Text.Wrap
            text: qsTr("Mentions")
        }

        NotificationTypeSetting {
            Layout.topMargin: 10
            filterablePref: notificationPrefs.mention
        }

        AccessibleText {
            Layout.preferredWidth: 120
            wrapMode: Text.Wrap
            text: qsTr("from")
        }

        NotificationIncludeSetting {
            filterablePref: notificationPrefs.mention
        }


        AccessibleText {
            Layout.topMargin: 10
            Layout.preferredWidth: 120
            wrapMode: Text.Wrap
            text: qsTr("Quotes")
        }

        NotificationTypeSetting {
            Layout.topMargin: 10
            filterablePref: notificationPrefs.quote
        }

        AccessibleText {
            Layout.preferredWidth: 120
            wrapMode: Text.Wrap
            text: qsTr("from")
        }

        NotificationIncludeSetting {
            filterablePref: notificationPrefs.quote
        }


        AccessibleText {
            Layout.topMargin: 10
            Layout.preferredWidth: 120
            wrapMode: Text.Wrap
            text: qsTr("Reposts")
        }

        NotificationTypeSetting {
            Layout.topMargin: 10
            filterablePref: notificationPrefs.repost
        }

        AccessibleText {
            Layout.preferredWidth: 120
            wrapMode: Text.Wrap
            text: qsTr("from")
        }

        NotificationIncludeSetting {
            filterablePref: notificationPrefs.repost
        }


        AccessibleText {
            Layout.topMargin: 10
            Layout.preferredWidth: 120
            wrapMode: Text.Wrap
            text: qsTr("Reposts of reposts")
        }

        NotificationTypeSetting {
            Layout.topMargin: 10
            filterablePref: notificationPrefs.repostViaRepost
        }

        AccessibleText {
            Layout.preferredWidth: 120
            wrapMode: Text.Wrap
            text: qsTr("from")
        }

        NotificationIncludeSetting {
            filterablePref: notificationPrefs.repostViaRepost
        }

        // TODO: chat, subscribed posts
    }

    AccessibleText {
        Layout.topMargin: 10
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        text: qsTr("Push notifications can be enabled/disabled in the app settings of your phone.")
    }

    AccessibleCheckBox {
        Layout.topMargin: 10
        text: qsTr("WiFi only")
        checked: userSettings.getNotificationsWifiOnly()
        onCheckedChanged: userSettings.setNotificationsWifiOnly(checked)
    }
}
