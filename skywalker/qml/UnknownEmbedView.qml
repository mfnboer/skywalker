import QtQuick
import QtQuick.Controls

Rectangle {
    required property string unknownEmbedType

    height: unknownEmbedText.height
    radius: guiSettings.radius
    border.width: 1
    border.color: guiSettings.borderColor
    color: "transparent"

    AccessibleText {
        id: unknownEmbedText
        width: parent.width
        padding: 20
        wrapMode: Text.Wrap
        text: `Unsupported embedded content: ${unknownEmbedType}`
    }
}
