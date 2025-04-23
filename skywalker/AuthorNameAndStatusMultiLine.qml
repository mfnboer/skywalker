import QtQuick
import skywalker

Rectangle {
    required property basicprofile author
    property double pointSize: guiSettings.scaledFont(1)
    property alias maximumLineCount: nameText.maximumLineCount
    property alias wrapMode: nameText.wrapMode
    property alias ellipsisBackgroundColor: nameText.ellipsisBackgroundColor
    readonly property bool authorVerified: author.verificationState.verifiedStatus === QEnums.VERIFIED_STATUS_VALID
    readonly property bool isTrustedVerifier: author.verificationState.trustedVerifierStatus === QEnums.VERIFIED_STATUS_VALID
    readonly property int badgeSize: guiSettings.verificationBadgeSize / guiSettings.scaledFont(1) * pointSize

    id: nameRect
    height: nameText.height
    color: "transparent"

    SkyCleanedText {
        id: nameText
        width: parent.width - (verificationStatusLoader.item ? verificationStatusLoader.item.width + 5 : 0) - (verifierStatusLoader.item ? verifierStatusLoader.item.width + 5 : 0)
        elide: Text.ElideRight
        font.bold: true
        font.pointSize: nameRect.pointSize
        color: guiSettings.textColor
        plainText: author.name
    }

    Loader {
        id: verificationStatusLoader
        active: authorVerified

        sourceComponent: VerifiedBadge {
            id: verifiedStatus
            x: nameText.contentWidth + 5
            y: (nameText.height - height) / 2
            width: badgeSize
            height: width
            author: nameRect.author
        }
    }

    Loader {
        id: verifierStatusLoader
        active: isTrustedVerifier

        sourceComponent: VerifierBadge {
            id: verifierStatus
            x: nameText.contentWidth + 5 + (authorVerified ? verificationStatusLoader.item.width + 5 : 0)
            y: (nameText.height - height) / 2
            width: badgeSize
            height: width
            author: nameRect.author
        }
    }
}
