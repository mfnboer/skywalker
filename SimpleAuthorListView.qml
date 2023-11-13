import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

ListView {
    required model
    property int rowPadding: 3
    property bool allowDelete: false

    signal authorClicked(basicprofile profile)
    signal deleteClicked(basicprofile profile)

    id: searchList
    spacing: 0
    boundsBehavior: Flickable.StopAtBounds
    clip: true
    flickDeceleration: guiSettings.flickDeceleration

    delegate: Rectangle {
        required property basicprofile modelData
        property alias author: authorEntry.modelData

        id: authorEntry
        width: searchList.width
        height: grid.height
        color: guiSettings.backgroundColor

        GridLayout {
            id: grid
            width: parent.width
            columns: 3
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
                    y: parent.y + rowPadding
                    width: parent.width - 12
                    height: width
                    avatarUrl: authorVisible() ? author.avatarUrl : ""
                    onClicked: authorClicked(author)
                }
            }

            Text {
                topPadding: rowPadding
                Layout.fillWidth: true
                elide: Text.ElideRight
                font.bold: true
                color: guiSettings.textColor
                text: author.name
            }

            SvgButton {
                id: deleteButton
                Layout.rowSpan: 2
                iconColor: guiSettings.textColor
                Material.background: "transparent"
                svg: svgOutline.delete
                onClicked: deleteClicked(author)
                visible: allowDelete && author.did
            }
            Rectangle {
                visible: !deleteButton.visible
            }

            Text {
                bottomPadding: rowPadding
                Layout.fillWidth: true
                Layout.fillHeight: true
                elide: Text.ElideRight
                font.pointSize: guiSettings.scaledFont(7/8)
                color: guiSettings.handleColor
                text: author.handle ? `@${author.handle}` : ""
            }

            Rectangle {
                Layout.columnSpan: 3
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: guiSettings.separatorColor
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
