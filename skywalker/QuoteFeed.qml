import QtQuick
import QtQuick.Layouts
import skywalker

Column {
    property generatorview feed

    id: quoteColumn
    padding: 10

    RowLayout {
        width: parent.width - 20

        FeedAvatar {
            id: avatar
            width: 24
            Layout.alignment: Qt.AlignTop
            avatarUrl: feed.avatar
        }

        Text {
            Layout.fillWidth: true
            elide: Text.ElideRight
            font.bold: true
            color: guiSettings.textColor
            text: feed.displayName
        }
    }

    Text {
        width: parent.width - 20
        wrapMode: Text.Wrap
        maximumLineCount: 5
        elide: Text.ElideRight
        textFormat: Text.RichText
        color: guiSettings.textColor
        text: feed.description
    }

    GuiSettings {
        id: guiSettings
    }
}
