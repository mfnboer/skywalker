import QtQuick
import skywalker

Column {
    required property int allowIncomingChat // QEnums::AllowIncomingChat

    SkyRoundRadioButton {
        width: parent.width
        padding: 0
        checked: allowIncomingChat === QEnums.ALLOW_INCOMING_CHAT_ALL
        text: qsTr("Everyone")
        onCheckedChanged: {
            if (checked)
                allowIncomingChat = QEnums.ALLOW_INCOMING_CHAT_ALL
        }
    }
    SkyRoundRadioButton {
        width: parent.width
        padding: 0
        checked: allowIncomingChat === QEnums.ALLOW_INCOMING_CHAT_FOLLOWING
        text: qsTr("Users I follow")
        onCheckedChanged: {
            if (checked)
                allowIncomingChat = QEnums.ALLOW_INCOMING_CHAT_FOLLOWING
        }
    }
    SkyRoundRadioButton {
        width: parent.width
        padding: 0
        checked: allowIncomingChat === QEnums.ALLOW_INCOMING_CHAT_NONE
        text: qsTr("No one")
        onCheckedChanged: {
            if (checked)
                allowIncomingChat = QEnums.ALLOW_INCOMING_CHAT_NONE
        }
    }
}
