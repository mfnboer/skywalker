import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

ColumnLayout {
    required property EditUserPreferences userPrefs
    required property EditChatNotificationPreferences chatNotificationPrefs
    readonly property int labelSize: width / 3

    id: messageColumn

    HeaderText {
        Layout.topMargin: 10
        text: qsTr("Direct messages")
    }

    AccessibleText {
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        text: qsTr("Allow new messages from:")
    }

    AllowIncomingChat {
        x: 10
        Layout.preferredWidth: parent.width - 20
        allowIncomingChat: userPrefs.allowIncomingChat

        onAllowIncomingChatChanged: userPrefs.allowIncomingChat = allowIncomingChat
    }

    AccessibleText {
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        color: guiSettings.textColor
        text: qsTr("Allow group invites from:")
    }

    AllowIncomingChat {
        x: 10
        Layout.preferredWidth: parent.width - 20
        allowIncomingChat: userPrefs.allowGroupInvites

        onAllowIncomingChatChanged: userPrefs.allowGroupInvites = allowIncomingChat
    }

    AccessibleText {
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        text: qsTr("Receive notifications:")
    }

    GridLayout {
        Layout.fillWidth: true
        columns: 2
        rowSpacing: 5

        AccessibleText {
            Layout.preferredWidth: labelSize
            wrapMode: Text.Wrap
            text: qsTr("Messages")
        }

        ChatNotificationTypeSetting {
            notificationPref: chatNotificationPrefs.chat
        }

        AccessibleText {
            Layout.preferredWidth: labelSize
            wrapMode: Text.Wrap
            text: qsTr("from")
        }

        NotificationIncludeSetting {
            filterablePref: chatNotificationPrefs.chat
        }


        AccessibleText {
            Layout.preferredWidth: labelSize
            wrapMode: Text.Wrap
            text: qsTr("Join requests")
        }

        ChatNotificationTypeSetting {
            notificationPref: chatNotificationPrefs.chatRequest
        }

        AccessibleText {
            Layout.preferredWidth: labelSize
            wrapMode: Text.Wrap
            text: qsTr("from")
        }

        NotificationIncludeSetting {
            filterablePref: chatNotificationPrefs.chatRequest
        }
    }
}
