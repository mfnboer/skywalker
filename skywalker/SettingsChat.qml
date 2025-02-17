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

    RadioButton {
        Layout.leftMargin: 10
        padding: 0
        checked: userPrefs.allowIncomingChat === QEnums.ALLOW_INCOMING_CHAT_ALL
        text: qsTr("Everyone")
        display: AbstractButton.TextOnly
        onCheckedChanged: {
            if (checked)
                userPrefs.allowIncomingChat = QEnums.ALLOW_INCOMING_CHAT_ALL
        }
    }
    RadioButton {
        Layout.leftMargin: 10
        padding: 0
        checked: userPrefs.allowIncomingChat === QEnums.ALLOW_INCOMING_CHAT_FOLLOWING
        text: qsTr("Users I follow")
        display: AbstractButton.TextOnly
        onCheckedChanged: {
            if (checked)
                userPrefs.allowIncomingChat = QEnums.ALLOW_INCOMING_CHAT_FOLLOWING
        }
    }
    RadioButton {
        Layout.leftMargin: 10
        padding: 0
        checked: userPrefs.allowIncomingChat === QEnums.ALLOW_INCOMING_CHAT_NONE
        text: qsTr("No one")
        display: AbstractButton.TextOnly
        onCheckedChanged: {
            if (checked)
                userPrefs.allowIncomingChat = QEnums.ALLOW_INCOMING_CHAT_NONE
        }
    }
}
