import QtQuick
import QtQuick.Layouts
import skywalker

RowLayout {
    required property profile author

    id: authorItem
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
            author: authorItem.author
        }
    }

    Column {
        Layout.fillWidth: true

        SkyCleanedText {
            width: parent.width
            elide: Text.ElideRight
            font.bold: true
            color: GuiSettings.textColor
            plainText: author.name
        }
        Text {
            width: parent.width
            elide: Text.ElideRight
            font.pointSize: GuiSettings.scaledFont(7/8)
            color: GuiSettings.handleColor
            text: `@${author.handle}`
        }
    }

}
