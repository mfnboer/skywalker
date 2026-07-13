import QtQuick
import skywalker

Rectangle {
    property string userDid
    required property basicprofile author
    property string authorName: author.name
    property int topPadding: 0
    property double pointSize: guiSettings.scaledFont(1)
    property alias maximumLineCount: nameText.maximumLineCount
    property alias wrapMode: nameText.wrapMode
    property alias ellipsisBackgroundColor: nameText.ellipsisBackgroundColor
    property Skywalker skywalker: root.getSkywalker(userDid)
    property var verificationUtils: skywalker.getVerificationUtils()
    property bool authorVerified: author.verificationState.verifiedStatus === QEnums.VERIFIED_STATUS_VALID
    property bool isTrustedVerifier: author.verificationState.trustedVerifierStatus === QEnums.VERIFIED_STATUS_VALID || verificationUtils.isVerifier(author.did)
    readonly property int badgeSize: guiSettings.verificationBadgeSize / guiSettings.scaledFont(1) * pointSize

    id: nameRect
    height: nameText.height
    color: "transparent"

    SkyCleanedText {
        id: nameText
        topPadding: nameRect.topPadding
        width: parent.width - (verificationStatusLoader.item ? verificationStatusLoader.item.width + 5 : 0) - (verifierStatusLoader.item ? verifierStatusLoader.item.width + 5 : 0)
        elide: Text.ElideRight
        font.bold: true
        font.pointSize: nameRect.pointSize
        color: guiSettings.textColor
        plainText: authorName
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
            author: nameRect.author
        }
    }

    Loader {
        id: verifierStatusLoader
        active: isTrustedVerifier && !skywalker.hideVerificationBadges

        sourceComponent: VerifierBadge {
            id: verifierStatus
            x: nameText.advanceWidth + 5 + (authorVerified ? verificationStatusLoader.item.width + 5 : 0)
            y: (nameText.height - height) / 2
            width: badgeSize
            height: width
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
