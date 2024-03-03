import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Page {
    signal closed()

    width: parent.width
    height: parent.height
    background: Rectangle { color: guiSettings.skywalkerLogoColor }

    Accessible.role: Accessible.Pane

    Column {
        width: parent.width
        height: parent.height

        AccessibleText {
            id: title
            anchors.horizontalCenter: parent.horizontalCenter
            padding: 10
            color: "white"
            font.bold: true
            font.pointSize: guiSettings.scaledFont(3.5)
            text: "Skywalker"
        }
        AccessibleText {
            id: version
            anchors.horizontalCenter: parent.horizontalCenter
            padding: 10
            color: "white"
            text: qsTr("Version") + ": " + skywalker.VERSION
        }
        AccessibleText {
            id: author
            anchors.horizontalCenter: parent.horizontalCenter
            padding: 10
            color: "white"
            text: "\u00A9 2024 Michel de Boer"
        }
        Text {
            id: handle
            anchors.horizontalCenter: parent.horizontalCenter
            padding: 10
            textFormat: Text.RichText
            text: `<a href=\"did:plc:zzmeflm2wzrrgcaam6bw3kaf\" style=\"color: ivory; text-decoration: none\">${guiSettings.skywalkerHandle}</a>`
            onLinkActivated: (link) => skywalker.getDetailedProfile(link)

            Accessible.role: Accessible.Link
            Accessible.name: `${guiSettings.skywalkerHandle}`
            Accessible.onPressAction: skywalker.getDetailedProfile("did:plc:zzmeflm2wzrrgcaam6bw3kaf")
        }
        Image {
            anchors.horizontalCenter: parent.horizontalCenter
            width: height
            height: Math.min(parent.height - title.height - version.height - author.height - handle.height - warranty.height - okButton.height - whitespace.height, parent.width)
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
            id: okButton
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("OK")
            onClicked: closed()

            Accessible.role: Accessible.Button
            Accessible.name: text
            Accessible.onPressAction: clicked()
        }
        Rectangle {
            id: whitespace
            width: 10
            height: 10
            color: "transparent"
        }
    }

    GuiSettings {
        id: guiSettings
    }
}
