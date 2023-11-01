import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    signal signIn()

    background: Rectangle { color: "#0387c7" }

    ColumnLayout {
        width: parent.width

        Text {
            Layout.alignment: Layout.Center
            padding: 10
            color: "white"
            font.bold: true
            font.pointSize: guiSettings.scaledFont(4)
            text: "Skywalker"
        }
        Text {
            Layout.fillWidth: true
            padding: 10
            wrapMode: Text.Wrap
            elide: Text.ElideRight
            color: "white"
            textFormat: Text.RichText
            text: qsTr("Explore the <b>Bluesky</b> social media network with Skywalker. To use Skywalker you must have a Bluesky account. If you do not have an account yet, then create one using the official Bluesky app or on <a href=\"https://bsky.app/\" style=\"color: ivory; font-weight: bold;\">https://bsky.app</a>")

            onLinkActivated: (link) => { root.openLink(link) }
        }
        Image {
            Layout.fillWidth: true
            fillMode: Image.PreserveAspectFit
            source: "images/skywalker.png"
        }
        SkyButton {
            Layout.alignment: Layout.Center
            text: qsTr("Sign In")
            onClicked: signIn()
        }
    }

    GuiSettings {
        id: guiSettings
    }

}
