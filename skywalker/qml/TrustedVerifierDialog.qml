import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import skywalker

Dialog {
    required property basicprofile author

    id: page
    width: parent.width - 40
    contentHeight: nameText.height + handleText.height + verifiedByText.height + 10
    modal: true
    standardButtons: Dialog.Ok
    anchors.centerIn: parent
    Material.background: guiSettings.backgroundColor

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
        text: qsTr("Verifiers can verify other accounts. Trusted verifiers are selected by Bluesky.")
    }
}
