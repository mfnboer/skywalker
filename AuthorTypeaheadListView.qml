import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

ListView {
    required model

    id: searchList
    spacing: 5

    delegate: GridLayout {
        required property basicprofile modelData
        property alias author: authorEntry.modelData

        id: authorEntry
        width: searchList.width
        columns: 2
        rowSpacing: 5
        columnSpacing: 10

        // Avatar
        Rectangle {
            id: avatar
            Layout.rowSpan: 2
            width: 50
            height: avatarImg.height
            Layout.fillHeight: true
            color: "transparent"

            Avatar {
                id: avatarImg
                x: parent.x + 8
                y: parent.y + 5
                width: parent.width - 13
                height: width
                avatarUrl: authorVisible() ? author.avatarUrl : ""
            }
        }

        Text {
            Layout.fillWidth: true
            topPadding: 5
            elide: Text.ElideRight
            font.bold: true
            text: author.name
        }

        Text {
            Layout.fillWidth: true
            Layout.fillHeight: true
            elide: Text.ElideRight
            font.pointSize: guiSettings.scaledFont(7/8)
            color: guiSettings.handleColor
            text: `@${author.handle}`
        }

        Rectangle {
            Layout.topMargin: 5
            Layout.columnSpan: 2
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: "lightgrey"
        }

        function authorVisible()
        {
            let visibility = skywalker.getContentVisibility(author.labels)
            return visibility === QEnums.CONTENT_VISIBILITY_SHOW
        }
    }

    GuiSettings {
        id: guiSettings
    }
}
