import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyPage {
    signal signIn()

    width: parent.width
    height: parent.height
    background: Rectangle { color: GuiSettings.skywalkerLogoColor }

    Accessible.role: Accessible.Pane
    Accessible.name: "Skywalker"

    Column {
        width: parent.width
        height: parent.height
        Accessible.role: Accessible.Pane

        AccessibleText {
            id: title
            anchors.horizontalCenter: parent.horizontalCenter
            padding: 10
            color: "white"
            font.bold: true
            font.pointSize: GuiSettings.scaledFont(3.5)
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

            Accessible.role: Accessible.StaticText
            Accessible.name: unicodeFonts.toPlainText(text)
        }
        Image {
            anchors.horizontalCenter: parent.horizontalCenter
            width: height
            height: Math.min(parent.height - title.height - description.height - warranty.height - signInButton.height - whitespace.height, parent.width)
            fillMode: Image.PreserveAspectFit
            source: "/images/skywalker.png"

            Accessible.ignored: true
        }
        AccessibleText {
            id: warranty
            width: parent.width
            padding: 10
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            color: "white"
            text: qsTr("Skywalker comes with ABSOLUTELY NO WARRANTY.")
        }
        SkyButton {
            id: signInButton
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Sign In")
            onClicked: signIn()
        }
        Rectangle {
            id: whitespace
            width: 10
            height: 10
            color: "transparent"
            Accessible.ignored: true
        }
    }

    UnicodeFonts {
        id: unicodeFonts
    }

}
