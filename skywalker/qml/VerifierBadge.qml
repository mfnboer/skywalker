import QtQuick
import skywalker

Image {
    property string userDid
    required property basicprofile author

    id: badge
    fillMode: Image.PreserveAspectFit
    source: author.verificationState.trustedVerifierStatus === QEnums.VERIFIED_STATUS_VALID ? "/images/verifier_check.svg" : "/images/verifier_check_pink.svg"
    asynchronous: true

    SkyMouseArea {
        anchors.fill: parent
        onClicked: showVerifierStatus()
    }

    function showVerifierStatus() {
        let component = guiSettings.createComponent("TrustedVerifierDialog.qml")
        let dialog = component.createObject(rootContent, {
                userDid: badge.userDid,
                author: badge.author })
        dialog.onAccepted.connect(() => { dialog.close() })
        dialog.onRejected.connect(() => { dialog.close() })
        dialog.open()
    }
}
