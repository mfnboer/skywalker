import QtQuick
import QtQuick.Controls.Material
import skywalker

Column {
    required property basicprofile author
    required property int postIndexedSecondsAgo

    Row {
        spacing: 10
        width: parent.width

        SkyCleanedText {
            width: parent.width - durationText.width - parent.spacing
            elide: Text.ElideRight
            plainText: author.name
            font.bold: true
            color: guiSettings.textColor

            Accessible.ignored: true
        }
        Text {
            id: durationText
            text: guiSettings.durationToString(postIndexedSecondsAgo)
            font.pointSize: guiSettings.scaledFont(7/8)
            color: Material.color(Material.Grey)

            Accessible.ignored: true
        }
    }

    Text {
        width: parent.width
        bottomPadding: 5
        elide: Text.ElideRight
        text: "@" + author.handle
        font.pointSize: guiSettings.scaledFont(7/8)
        color: guiSettings.handleColor

        Accessible.ignored: true
    }

    Rectangle {
        width: parent.width
        height: author.labels ? contentLabels.height + 5 : 0
        color: "transparent"

        ContentLabels {
            id: contentLabels
            anchors.left: parent.left
            anchors.right: undefined
            contentLabels: author.labels
            contentAuthorDid: author.did
        }
    }

    GuiSettings {
        id: guiSettings
    }
}
