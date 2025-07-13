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

    AccessibleText {
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        color: guiSettings.textColor
        text: qsTr("Allow others to be notified of your posts:")
    }

    SkyRoundRadioButton {
        Layout.leftMargin: 10
        padding: 0
        checked: notificationPrefs.allowSubscriptions === QEnums.ALLOW_ACTIVITY_SUBSCRIPTIONS_FOLLOWERS
        text: qsTr("Anyone who follows me")
        onCheckedChanged: {
            if (checked)
                notificationPrefs.allowSubscriptions = QEnums.ALLOW_ACTIVITY_SUBSCRIPTIONS_FOLLOWERS
        }
    }
    SkyRoundRadioButton {
        Layout.leftMargin: 10
        padding: 0
        checked: notificationPrefs.allowSubscriptions === QEnums.ALLOW_ACTIVITY_SUBSCRIPTIONS_MUTUALS
        text: qsTr("Only followers who I follow")
        onCheckedChanged: {
            if (checked)
                notificationPrefs.allowSubscriptions = QEnums.ALLOW_ACTIVITY_SUBSCRIPTIONS_MUTUALS
        }
    }
    SkyRoundRadioButton {
        Layout.leftMargin: 10
        padding: 0
        checked: notificationPrefs.allowSubscriptions === QEnums.ALLOW_ACTIVITY_SUBSCRIPTIONS_NONE
        text: qsTr("No one")
        onCheckedChanged: {
            if (checked)
                notificationPrefs.allowSubscriptions = QEnums.ALLOW_ACTIVITY_SUBSCRIPTIONS_NONE
        }
    }


    AccessibleText {
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        color: guiSettings.textColor
        text: qsTr("Receive notifications:")
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
            notificationPref: notificationPrefs.like
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
            notificationPref: notificationPrefs.likeViaRepost
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
            notificationPref: notificationPrefs.follow
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
            notificationPref: notificationPrefs.reply
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
            notificationPref: notificationPrefs.mention
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
            notificationPref: notificationPrefs.quote
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
            notificationPref: notificationPrefs.repost
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
            notificationPref: notificationPrefs.repostViaRepost
        }

        AccessibleText {
            Layout.preferredWidth: 120
            wrapMode: Text.Wrap
            text: qsTr("from")
        }

        NotificationIncludeSetting {
            filterablePref: notificationPrefs.repostViaRepost
        }


        AccessibleText {
            Layout.topMargin: 10
            Layout.preferredWidth: 120
            wrapMode: Text.Wrap
            text: qsTr("Subscribed posts")
        }

        NotificationTypeSetting {
            Layout.topMargin: 10
            notificationPref: notificationPrefs.subscribedPost
        }

        AccessibleText {
            Layout.topMargin: 10
            Layout.columnSpan: 2
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            textFormat: Text.RichText
            text: qsTr(`<a href="link" style="color: ${guiSettings.linkColor}; text-decoration: none">Show subscriptions</a>`)
            onLinkActivated: {
                let modelId = skywalker.createAuthorListModel(QEnums.AUTHOR_LIST_ACTIVITY_SUBSCRIPTIONS, "")
                root.viewAuthorList(modelId, qsTr("Subscribed accounts"), "", false, true)
            }
        }
    }

    AccessibleText {
        Layout.topMargin: 10
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        text: qsTr("Push notifications can be enabled/disabled in the app settings of your phone.")
    }

    AccessibleCheckBox {
        Layout.topMargin: 10
        text: qsTr("Push notifications on WiFi only")
        checked: userSettings.getNotificationsWifiOnly()
        onCheckedChanged: userSettings.setNotificationsWifiOnly(checked)
    }
}
