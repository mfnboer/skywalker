import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

ListView {
    required model

    signal authorClicked(basicprofile profile)

    id: searchList
    spacing: 3

    delegate: Rectangle {
        required property basicprofile modelData
        property alias author: authorEntry.modelData

        id: authorEntry
        width: searchList.width
        height: grid.height

        GridLayout {
            id: grid
            width: parent.width
            columns: 2
            rowSpacing: 0
            columnSpacing: 10

            // Avatar
            Rectangle {
                id: avatar
                Layout.rowSpan: 2
                width: 44
                height: avatarImg.height
                Layout.fillHeight: true
                color: "transparent"

                Avatar {
                    id: avatarImg
                    x: parent.x + 8
                    y: parent.y
                    width: parent.width - 13
                    height: width
                    avatarUrl: authorVisible() ? author.avatarUrl : ""
                    onClicked: authorClicked(author)
                }
            }

            Text {
                Layout.fillWidth: true
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
                Layout.topMargin: 3
                Layout.columnSpan: 2
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: "lightgrey"
            }
        }
        MouseArea {
            z: -1
            anchors.fill: parent
            onClicked: authorClicked(author)
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
