import QtQuick
import skywalker

Image {
    property string userDid
    required property basicprofile author

    id: badge
    fillMode: Image.PreserveAspectFit
    source: author.verificationState.verifiedStatus === QEnums.VERIFIED_STATUS_VALID ? "/images/verified_check.svg" : "/images/verified_check_pink.svg"
    asynchronous: true

    SkyMouseArea {
        anchors.fill: parent
        onClicked: showVerifiedStatus()
    }

    function showVerifiedStatus() {
        let component = guiSettings.createComponent("VerifiedDialog.qml")
        let dialog = component.createObject(rootContent, { author: badge.author })
        dialog.onAccepted.connect(() => { dialog.close() })
        dialog.onRejected.connect(() => { dialog.close() })
        dialog.open()
    }
}
