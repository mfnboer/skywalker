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
            width: 34
            Layout.alignment: Qt.AlignTop
            avatarUrl: feed.avatar

            onClicked: skywalker.getFeedGenerator(feed.uri)
        }

        Column {
            Layout.fillWidth: true

            Text {
                elide: Text.ElideRight
                font.bold: true
                color: guiSettings.textColor
                text: feed.displayName
            }

            Text {
                elide: Text.ElideRight
                font.pointSize: guiSettings.scaledFont(7/8)
                color: guiSettings.handleColor
                text: qsTr(`feed by @${feed.creator.handle}`)
            }
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
