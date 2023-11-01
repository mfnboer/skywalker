import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    signal signIn()

    width: parent.width
    height: parent.height
    background: Rectangle { color: "#0387c7" }

    Column {
        width: parent.width
        height: parent.height

        Text {
            id: title
            anchors.horizontalCenter: parent.horizontalCenter
            padding: 10
            color: "white"
            font.bold: true
            font.pointSize: guiSettings.scaledFont(4)
            text: "Skywalker"
        }
        Text {
            id: description
            width: parent.width
            padding: 10
            wrapMode: Text.Wrap
            elide: Text.ElideRight
            color: "white"
            textFormat: Text.RichText
            text: qsTr("Explore the <b>Bluesky</b> social media network with Skywalker. To use Skywalker you must have a Bluesky account. If you do not have an account yet, then create one using the official Bluesky app or on <a href=\"https://bsky.app/\" style=\"color: ivory; font-weight: bold;\">https://bsky.app</a>")

            onLinkActivated: (link) => { root.openLink(link) }
        }
        Image {
            anchors.horizontalCenter: parent.horizontalCenter
            width: height
            height: Math.min(parent.height - title.height - description.height - signInButton.height, parent.width)
            fillMode: Image.PreserveAspectFit
            source: "images/skywalker.png"
        }
        SkyButton {
            id: signInButton
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Sign In")
            onClicked: signIn()
        }
    }

    GuiSettings {
        id: guiSettings
    }

}
