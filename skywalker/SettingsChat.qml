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

    SkyRoundRadioButton {
        Layout.leftMargin: 10
        padding: 0
        checked: userPrefs.allowIncomingChat === QEnums.ALLOW_INCOMING_CHAT_ALL
        text: qsTr("Everyone")
        onCheckedChanged: {
            if (checked)
                userPrefs.allowIncomingChat = QEnums.ALLOW_INCOMING_CHAT_ALL
        }
    }
    SkyRoundRadioButton {
        Layout.leftMargin: 10
        padding: 0
        checked: userPrefs.allowIncomingChat === QEnums.ALLOW_INCOMING_CHAT_FOLLOWING
        text: qsTr("Users I follow")
        onCheckedChanged: {
            if (checked)
                userPrefs.allowIncomingChat = QEnums.ALLOW_INCOMING_CHAT_FOLLOWING
        }
    }
    SkyRoundRadioButton {
        Layout.leftMargin: 10
        padding: 0
        checked: userPrefs.allowIncomingChat === QEnums.ALLOW_INCOMING_CHAT_NONE
        text: qsTr("No one")
        onCheckedChanged: {
            if (checked)
                userPrefs.allowIncomingChat = QEnums.ALLOW_INCOMING_CHAT_NONE
        }
    }
}
