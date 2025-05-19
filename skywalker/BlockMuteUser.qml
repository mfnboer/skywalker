import QtQuick
import QtQuick.Controls
import skywalker

Dialog {
    required property bool blockUser
    required property string handle
    property alias expiresAt: durationInput.expiresAt

    id: page
    width: parent.width
    contentHeight: durationInput.y + durationInput.height
    topMargin: guiSettings.headerHeight
    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel
    Material.background: guiSettings.backgroundColor

    AccessibleText {
        id: headerText
        width: parent.width
        elide: Text.ElideRight
        text: (blockUser ? qsTr("Block") : qsTr("Mute")) + ": " + handle
    }

    DurationInput {
        id: durationInput
        anchors.top: headerText.bottom
        width: parent.width
    }
}
