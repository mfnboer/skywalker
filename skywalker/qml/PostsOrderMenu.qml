import QtQuick
import QtQuick.Controls
import skywalker

SkyMenu {
    required property bool reverseFeed
    required property int globalFeedOrder // QEnums::FeedOrder

    signal newReverseFeed(bool reverse)

    title: qsTr("Posts order")

    CloseMenuItem {
        text: qsTr("<b>Posts order</b>")
        Accessible.name: qsTr("close posts order menu")
    }

    SkyRadioMenuItem {
        text: qsTr("New to old")
        checked: !reverseFeed
        enabled: globalFeedOrder === QEnums.FEED_ORDER_PER_FEED
        onTriggered: newReverseFeed(false)
    }

    SkyRadioMenuItem {
        text: qsTr("Old to new")
        checked: reverseFeed
        enabled: globalFeedOrder === QEnums.FEED_ORDER_PER_FEED
        onTriggered: newReverseFeed(true)
    }
}
