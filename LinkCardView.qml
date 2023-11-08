import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

RoundedFrame {
    property string uri
    property string title
    property string description
    property string thumbUrl
    property int columnHeight: externalColumn.height

    id: card
    objectToRound: externalColumn
    border.width: 1
    border.color: guiSettings.borderColor

    Column {
        id: externalColumn
        width: parent.width
        spacing: 5
        visible: false

        ImageAutoRetry {
            id: thumbImg
            width: parent.width
            source: card.thumbUrl
            fillMode: Image.PreserveAspectFit
        }
        Text {
            id: linkText
            width: parent.width - 10
            leftPadding: 5
            rightPadding: 5
            text: card.uri ? new URL(card.uri).hostname : ""
            elide: Text.ElideRight
            color: guiSettings.linkColor
        }
        Text {
            id: titleText
            width: parent.width - 10
            leftPadding: 5
            rightPadding: 5
            color: Material.foreground
            text: card.title
            wrapMode: Text.Wrap
            maximumLineCount: 2
            elide: Text.ElideRight
            font.bold: true
        }
        Text {
            id: descriptionText
            width: parent.width - 10
            leftPadding: 5
            rightPadding: 5
            bottomPadding: 5
            color: Material.foreground
            text: card.description ? card.description : card.uri
            wrapMode: Text.Wrap
            maximumLineCount: 5
            elide: Text.ElideRight
        }
    }

    GuiSettings {
        id: guiSettings
    }
}
