import QtQuick
import QtQuick.Layouts
import skywalker

RowLayout {
    required property profile author

    width: parent.width
    spacing: 10

    // Avatar
    Rectangle {
        width: 50
        Layout.fillHeight: true
        color: "transparent"

        Avatar {
            x: 10
            anchors.verticalCenter: parent.verticalCenter
            width: parent.width - 13
            height: width
            avatarUrl: author.avatarUrl
            isModerator: author.associated.isLabeler
        }
    }

    Column {
        Layout.fillWidth: true

        SkyCleanedText {
            width: parent.width
            elide: Text.ElideRight
            font.bold: true
            color: guiSettings.textColor
            plainText: author.name
        }
        Text {
            width: parent.width
            elide: Text.ElideRight
            font.pointSize: guiSettings.scaledFont(7/8)
            color: guiSettings.handleColor
            text: `@${author.handle}`
        }
    }

    GuiSettings {
        id: guiSettings
    }
}
