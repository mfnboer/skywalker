import QtQuick
import skywalker

Rectangle {
    property string userDid
    required property basicprofile author
    property double pointSize: guiSettings.scaledFont(1)
    property string textColor: guiSettings.textColor
    property bool authorVerified: author.verificationState.verifiedStatus === QEnums.VERIFIED_STATUS_VALID
    property Skywalker skywalker: root.getSkywalker(userDid)
    property var verificationUtils: skywalker.getVerificationUtils()
    readonly property bool isTrustedVerifier: author.verificationState.trustedVerifierStatus === QEnums.VERIFIED_STATUS_VALID || verificationUtils.isVerifier(author.did)
    readonly property int badgeSize: guiSettings.verificationBadgeSize / guiSettings.scaledFont(1) * pointSize
    readonly property int verificationStatusWidth: (verificationStatusLoader.item ? verificationStatusLoader.item.width + 5 : 0) - (verifierStatusLoader.item ? verifierStatusLoader.item.width + 5 : 0)
    readonly property real advanceWidth: nameText.advanceWidth + verificationStatusWidth

    id: nameRect
    height: nameText.height
    color: "transparent"

    SkyCleanedTextLine {
        id: nameText
        width: parent.width - verificationStatusWidth
        elide: Text.ElideRight
        font.bold: true
        font.pointSize: nameRect.pointSize
        color: nameRect.textColor
        plainText: author.name
    }

    function getNameText() {
        return nameText
    }

    Loader {
        id: verificationStatusLoader
        active: authorVerified && !skywalker.hideVerificationBadges

        sourceComponent: VerifiedBadge {
            id: verifiedStatus
            x: nameText.advanceWidth + 5
            y: (nameText.height - height) / 2
            width: badgeSize
            height: width
            userDid: nameRect.userDid
            author: nameRect.author
        }
    }

    Loader {
        id: verifierStatusLoader
        active: !skywalker.hideVerificationBadges && isTrustedVerifier

        sourceComponent: VerifierBadge {
            id: verifierStatus
            x: nameText.advanceWidth + 5 + (authorVerified ? verificationStatusLoader.item?.width + 5 : 0)
            y: (nameText.height - height) / 2
            width: badgeSize
            height: width
            userDid: nameRect.userDid
            author: nameRect.author
        }
    }

    Connections {
        target: verificationUtils

        function onVerified(did, isVerified) {
            if (did !== author.did)
                return

            authorVerified = isVerified
        }
    }

    Component.onCompleted: {
        if (!skywalker.hideVerificationBadges && !authorVerified)
            verificationUtils.isVerified(author)
    }
}
