import QtQuick
import QtQuick.Layouts
import skywalker

ColumnLayout {
    property var skywalker: root.getSkywalker()
    property var userSettings: skywalker.getUserSettings()
    property string userDid: userSettings.getActiveUserDid()

    HeaderText {
        Layout.topMargin: 10
        text: qsTr("Post threads")
    }

    AccessibleText {
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        text: qsTr("Sort replies to a post in a post thread:")
    }

    SkyRoundRadioButton {
        Layout.leftMargin: 10
        padding: 0
        checked: userSettings.getThreadReplyOrder(userDid) === QEnums.REPLY_ORDER_SMART
        text: qsTr("Smart")
        onCheckedChanged: {
            if (checked)
                userSettings.setThreadReplyOrder(userDid, QEnums.REPLY_ORDER_SMART)
        }
    }
    SkyRoundRadioButton {
        Layout.leftMargin: 10
        padding: 0
        checked: userSettings.getThreadReplyOrder(userDid) === QEnums.REPLY_ORDER_OLDEST_FIRST
        text: qsTr("Oldest reply first")
        onCheckedChanged: {
            if (checked)
                userSettings.setThreadReplyOrder(userDid, QEnums.REPLY_ORDER_OLDEST_FIRST)
        }
    }
    SkyRoundRadioButton {
        Layout.leftMargin: 10
        padding: 0
        checked: userSettings.getThreadReplyOrder(userDid) === QEnums.REPLY_ORDER_NEWEST_FIRST
        text: qsTr("Newest reply first")
        onCheckedChanged: {
            if (checked)
                userSettings.setThreadReplyOrder(userDid, QEnums.REPLY_ORDER_NEWEST_FIRST)
        }
    }
    SkyRoundRadioButton {
        Layout.leftMargin: 10
        padding: 0
        checked: userSettings.getThreadReplyOrder(userDid) === QEnums.REPLY_ORDER_MOST_LIKES_FIRST
        text: qsTr("Most liked reply first")
        onCheckedChanged: {
            if (checked)
                userSettings.setThreadReplyOrder(userDid, QEnums.REPLY_ORDER_MOST_LIKES_FIRST)
        }
    }
}
