import QtQuick
import QtQuick.Controls

SkyMenu {
    required property bool reverseFeed

    title: qsTr("Posts order")

    CloseMenuItem {
        text: qsTr("<b>Posts order</b>")
        Accessible.name: qsTr("close posts order menu")
    }

    SkyRadioMenuItem {
        text: qsTr("New to old")
        checked: !reverseFeed
        onTriggered: reverseFeed = false
    }

    SkyRadioMenuItem {
        text: qsTr("Old to new")
        checked: reverseFeed
        onTriggered: reverseFeed = true
    }
}
