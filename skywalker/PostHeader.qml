import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

GridLayout {
    required property basicprofile author
    required property int postIndexedSecondsAgo
    required property int postThreadType

    columns: 2
    rowSpacing: 0

    SkyCleanedText {
        Layout.fillWidth: true
        elide: Text.ElideRight
        plainText: author.name
        font.bold: true
        color: guiSettings.textColor

        Accessible.ignored: true
    }
    Text {
        text: guiSettings.durationToString(postIndexedSecondsAgo)
        font.pointSize: guiSettings.scaledFont(7/8)
        color: Material.color(Material.Grey)

        Accessible.ignored: true
    }

    Text {
        Layout.columnSpan: 2
        Layout.fillWidth: true
        bottomPadding: 5
        elide: Text.ElideRight
        text: "@" + author.handle
        font.pointSize: guiSettings.scaledFont(7/8)
        color: guiSettings.handleColor

        Accessible.ignored: true
    }

    Rectangle {
        Layout.columnSpan: 2
        Layout.fillWidth: true
        Layout.preferredHeight: author.labels ? contentLabels.height + 5 : 0
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
