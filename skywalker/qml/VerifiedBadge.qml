import QtQuick
import skywalker

Image {
    required property basicprofile author

    id: badge
    fillMode: Image.PreserveAspectFit
    source: "/images/verified_check.svg"

    MouseArea {
        anchors.fill: parent
        onClicked: showVerifiedStatus()
    }

    function showVerifiedStatus() {
        let component = guiSettings.createComponent("VerifiedDialog.qml")
        let dialog = component.createObject(rootContent, { author: badge.author })
        dialog.onAccepted.connect(() => { dialog.destroy() })
        dialog.onRejected.connect(() => { dialog.destroy() })
        dialog.open()
    }
}
