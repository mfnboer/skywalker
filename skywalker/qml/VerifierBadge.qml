import QtQuick
import skywalker

Image {
    required property basicprofile author

    id: badge
    fillMode: Image.PreserveAspectFit
    source: "/images/verifier_check.svg"
    asynchronous: true

    SkyMouseArea {
        anchors.fill: parent
        onClicked: showVerifierStatus()
    }

    function showVerifierStatus() {
        let component = guiSettings.createComponent("TrustedVerifierDialog.qml")
        let dialog = component.createObject(rootContent, { author: badge.author })
        dialog.onAccepted.connect(() => { dialog.destroy() })
        dialog.onRejected.connect(() => { dialog.destroy() })
        dialog.open()
    }
}
