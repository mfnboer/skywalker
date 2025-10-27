import QtQuick
import QtQuick.Layouts
import skywalker

RowLayout {
    required property basicprofile author

    signal acceptConvo
    signal deleteConvo
    signal blockAndDeleteConvo

    SkyButton {
        Layout.fillWidth: true
        text: qsTr("Accept")
        onClicked: acceptConvo()
    }
    SkyButton {
        Layout.fillWidth: true
        text: qsTr("Delete")
        onClicked: deleteConvo()
    }
    SkyButton {
        Layout.fillWidth: true
        text: qsTr("Block")
        visible: !author.viewer.blocking && !root.isActiveUser(author.did)
        onClicked: blockAndDeleteConvo()
    }
}
