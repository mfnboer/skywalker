import QtQuick
import QtQuick.Controls
import skywalker

SkyDialog {
    property string userDid
    required property basicprofile author
    property Skywalker skywalker: root.getSkywalker(userDid)
    property var verificationUtils: skywalker.getVerificationUtils()
    readonly property string verifierReason: verificationUtils.isVerifier(author.did) ?
                                                 qsTr("This verifier is trusted by you.") :
                                                 qsTr("This verifier is trusted by Bluesky.")

    id: page
    width: parent.width - 40
    contentHeight: nameText.height + handleText.height + verifiedByText.height + viewText.height + 10
    standardButtons: Dialog.Ok
    anchors.centerIn: parent

    SkyCleanedText {
        id: nameText
        x: 10
        width: parent.width - 20
        wrapMode: Text.Wrap
        font.bold: true
        font.pointSize: guiSettings.scaledFont(10/8)
        color: guiSettings.textColor
        plainText: qsTr(`${author.name} is a trusted verifier`)
    }

    AccessibleText {
        id: handleText
        x: 10
        anchors.top: nameText.bottom
        width: parent.width - 20
        elide: Text.ElideRight
        color: guiSettings.handleColor
        text: `@${author.handle}`
    }

    AccessibleText {
        id: verifiedByText
        x: 10
        anchors.top: handleText.bottom
        anchors.topMargin: 10
        width: parent.width - 20
        wrapMode: Text.Wrap
        text: qsTr(`Verifiers can verify other accounts. ${verifierReason}`)
    }

    AccessibleText {
        id: viewText
        x: 10
        width: parent.width - 20
        anchors.top: verifiedByText.bottom
        anchors.topMargin: 10
        horizontalAlignment: Text.AlignHCenter
        elide: Text.ElideRight
        textFormat: Text.RichText
        text: qsTr(`<a href="settings" style="color: ${guiSettings.linkColor}; text-decoration: none">View trusted verifiers</a>.`)
        onLinkActivated: {
            root.viewTrustedVerifiers()
            page.accept()
        }
    }
}
