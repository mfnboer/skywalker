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

    Accessible.role: Accessible.List

    delegate: Rectangle {
        required property basicprofile modelData
        property alias author: authorEntry.modelData

        id: authorEntry
        width: searchList.width
        height: grid.height
        color: guiSettings.backgroundColor

        Accessible.role: Accessible.Button
        Accessible.name: author.name
        Accessible.onPressAction: authorClicked(author)

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
                height: avatarImg.height + rowPadding + 2
                Layout.fillHeight: true
                color: "transparent"

                Accessible.ignored: true

                Avatar {
                    id: avatarImg
                    x: 8
                    y: rowPadding + 2
                    width: parent.width - 12
                    height: width
                    avatarUrl: guiSettings.contentVisible(author) ? author.avatarUrl : ""
                    isModerator: author.associated.isLabeler
                    onClicked: authorClicked(author)
                }
            }

            SkyCleanedText {
                topPadding: rowPadding
                Layout.fillWidth: true
                elide: Text.ElideRight
                font.bold: true
                color: guiSettings.textColor
                plainText: author.name

                Accessible.ignored: true
            }

            SvgButton {
                id: deleteButton
                Layout.rowSpan: 2
                iconColor: guiSettings.textColor
                Material.background: "transparent"
                svg: svgOutline.delete
                accessibleName: qsTr(`delete ${author.name}`)
                onClicked: deleteClicked(author)
                visible: allowDelete && author.did
            }
            Rectangle {
                Layout.rowSpan: 2
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

                Accessible.ignored: true
            }

            Item{}

            Rectangle {
                Layout.columnSpan: 2
                Layout.fillWidth: true
                height: contentLabels.height + 5
                color: "transparent"

                ContentLabels {
                    id: contentLabels
                    anchors.left: parent.left
                    //anchors.leftMargin: 8
                    anchors.right: undefined
                    contentLabels: author.labels
                    contentAuthorDid: author.did
                }
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
    }

    GuiSettings {
        id: guiSettings
    }
}
