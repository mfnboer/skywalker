import QtQuick
import QtQuick.Layouts
import skywalker

ColumnLayout {
    property var skywalker: root.getSkywalker()
    property var userSettings: skywalker.getUserSettings()
    property string userDid: userSettings.getActiveUserDid()

    HeaderText {
        Layout.topMargin: 10
        Layout.bottomMargin: 10
        text: qsTr("General feed preferences")
    }

    AccessibleCheckBox {
        text: qsTr("Assemble post threads")
        checked: userSettings.getAssembleThreads(userDid)
        onCheckedChanged: userSettings.setAssembleThreads(userDid, checked)
    }

    AccessibleCheckBox {
        text: qsTr("Show quotes with blocked post")
        checked: userSettings.getShowQuotesWithBlockedPost(userDid)
        onCheckedChanged: userSettings.setShowQuotesWithBlockedPost(userDid, checked)
    }

    AccessibleText {
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        text: qsTr("Posts order:")
    }

    SkyRoundRadioButton {
        Layout.leftMargin: 10
        padding: 0
        checked: userSettings.globalFeedOrder === QEnums.FEED_ORDER_PER_FEED
        text: qsTr("Per feed setting")
        onCheckedChanged: {
            if (checked)
                userSettings.globalFeedOrder = QEnums.FEED_ORDER_PER_FEED
        }
    }

    SkyRoundRadioButton {
        Layout.leftMargin: 10
        padding: 0
        checked: userSettings.globalFeedOrder === QEnums.FEED_ORDER_NEW_TO_OLD
        text: qsTr("New to old")
        onCheckedChanged: {
            if (checked)
                userSettings.globalFeedOrder = QEnums.FEED_ORDER_NEW_TO_OLD
        }
    }

    SkyRoundRadioButton {
        Layout.leftMargin: 10
        padding: 0
        checked: userSettings.globalFeedOrder === QEnums.FEED_ORDER_OLD_TO_NEW
        text: qsTr("Old to new")
        onCheckedChanged: {
            if (checked)
                userSettings.globalFeedOrder = QEnums.FEED_ORDER_OLD_TO_NEW
        }
    }
}
