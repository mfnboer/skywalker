import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

ColumnLayout {
    required property var userPrefs

    id: messageColumn

    HeaderText {
        Layout.topMargin: 10
        text: qsTr("Direct messages")
    }

    AccessibleText {
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        color: guiSettings.textColor
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
}
