import QtQuick
import skywalker

Rectangle {
    property string userDid
    required property basicprofile author
    property double pointSize: guiSettings.scaledFont(1)
    readonly property bool authorVerified: author.verificationState.verifiedStatus === QEnums.VERIFIED_STATUS_VALID
    readonly property bool isTrustedVerifier: author.verificationState.trustedVerifierStatus === QEnums.VERIFIED_STATUS_VALID
    readonly property int badgeSize: guiSettings.verificationBadgeSize / guiSettings.scaledFont(1) * pointSize
    readonly property int verificationStatusWidth: (verificationStatusLoader.item ? verificationStatusLoader.item.width + 5 : 0) - (verifierStatusLoader.item ? verifierStatusLoader.item.width + 5 : 0)
    readonly property real advanceWidth: nameText.advanceWidth + verificationStatusWidth
    // TODO: factor out badge width

    id: nameRect
    height: nameText.height
    color: "transparent"

    SkyCleanedTextLine {
        id: nameText
        width: parent.width - verificationStatusWidth
        elide: Text.ElideRight
        font.bold: true
        font.pointSize: nameRect.pointSize
        color: guiSettings.textColor
        plainText: author.name
    }

    Loader {
        id: verificationStatusLoader
        active: authorVerified && !root.getSkywalker(userDid).hideVerificationBadges

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
        active: isTrustedVerifier && !root.getSkywalker(userDid).hideVerificationBadges

        sourceComponent: VerifierBadge {
            id: verifierStatus
            x: nameText.advanceWidth + 5 + (authorVerified ? verificationStatusLoader.item?.width + 5 : 0)
            y: (nameText.height - height) / 2
            width: badgeSize
            height: width
            author: nameRect.author
        }
    }
}
