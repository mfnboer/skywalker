import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Page {
    signal closed()

    width: parent.width
    height: parent.height
    background: Rectangle { color: guiSettings.skywalkerLogoColor }

    Column {
        width: parent.width
        height: parent.height

        Text {
            id: title
            anchors.horizontalCenter: parent.horizontalCenter
            padding: 10
            color: "white"
            font.bold: true
            font.pointSize: guiSettings.scaledFont(3.5)
            text: "Skywalker"
        }
        Text {
            id: version
            anchors.horizontalCenter: parent.horizontalCenter
            padding: 10
            color: "white"
            text: qsTr("Version") + ": " + skywalker.VERSION
        }
        Text {
            id: author
            anchors.horizontalCenter: parent.horizontalCenter
            padding: 10
            color: "white"
            text: "\u00A9 2023 Michel de Boer"
        }
        Text {
            id: handle
            anchors.horizontalCenter: parent.horizontalCenter
            padding: 10
            textFormat: Text.RichText
            text: "<a href=\"did:plc:zzmeflm2wzrrgcaam6bw3kaf\" style=\"color: ivory;\">@skywalkerapp.bsky.social</a>"
            onLinkActivated: (link) => skywalker.getDetailedProfile(link)
        }
        Image {
            anchors.horizontalCenter: parent.horizontalCenter
            width: height
            height: Math.min(parent.height - title.height - version.height - author.height - handle.height - okButton.height, parent.width)
            fillMode: Image.PreserveAspectFit
            source: "images/skywalker.png"
        }
        SkyButton {
            id: okButton
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("OK")
            onClicked: closed()
        }
    }

    GuiSettings {
        id: guiSettings
    }
}