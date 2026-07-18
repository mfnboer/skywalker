import QtQuick
import skywalker

Rectangle {
    property string userDid
    required property basicprofile author
    property string authorName: author.name
    property double pointSize: guiSettings.scaledFont(1)
    property string textColor: guiSettings.textColor
    property bool authorVerified: author.verificationState.verifiedStatus === QEnums.VERIFIED_STATUS_VALID
    property Skywalker skywalker: root.getSkywalker(userDid)
    property var verificationUtils: skywalker.getVerificationUtils()
    readonly property bool isTrustedVerifier: author.verificationState.trustedVerifierStatus === QEnums.VERIFIED_STATUS_VALID || verificationUtils.isVerifier(author.did)
    readonly property int badgeSize: guiSettings.verificationBadgeSize / guiSettings.scaledFont(1) * pointSize
    readonly property int verificationStatusWidth: skywalker.hideVerificationBadges ? 0 : (authorVerified ? badgeSize + 5 : 0) + (isTrustedVerifier ? badgeSize + 5 : 0)
    property alias maximumLineCount: nameText.maximumLineCount
    property alias wrapMode: nameText.wrapMode
    property alias topPadding: nameText.topPadding

    id: nameRect
    height: nameText.implicitHeight
    color: "transparent"

    AccessibleText {
        id: nameText
        width: parent.width - verificationStatusWidth
        elide: Text.ElideRight
        font.bold: true
        font.pointSize: nameRect.pointSize
        color: nameRect.textColor
        text: authorName
    }

    Loader {
        id: verificationStatusLoader
        x: Math.min(nameText.width, nameText.contentWidth) + 5
        anchors.verticalCenter: parent.verticalCenter
        active: authorVerified && !skywalker.hideVerificationBadges

        sourceComponent: VerifiedBadge {
            id: verifiedStatus
            width: badgeSize
            height: width
            userDid: nameRect.userDid
            author: nameRect.author
        }
    }

    Loader {
        id: verifierStatusLoader
        x: Math.min(nameText.width, nameText.contentWidth) + 5 + (authorVerified ? badgeSize + 5 : 0)
        anchors.verticalCenter: parent.verticalCenter
        active: !skywalker.hideVerificationBadges && isTrustedVerifier

        sourceComponent: VerifierBadge {
            id: verifierStatus
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
