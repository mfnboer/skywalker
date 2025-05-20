import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Dialog {
    required property bool blockUser
    required property basicprofile author
    property alias expiresAt: durationInput.expiresAt

    id: page
    width: parent.width
    contentHeight: durationInput.y + durationInput.height
    topMargin: guiSettings.headerHeight
    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel
    Material.background: guiSettings.backgroundColor

    AccessibleText {
        id: headerText
        width: parent.width
        elide: Text.ElideRight
        font.bold: true
        text: (blockUser ? qsTr("Block") : qsTr("Mute")) + ":"
    }

    GridLayout {
        id: authorGrid
        anchors.top: headerText.bottom
        anchors.topMargin: 10
        columns: 3
        width: parent.width
        rowSpacing: 10
        columnSpacing: 10

        // Avatar
        Rectangle {
            id: avatar
            Layout.rowSpan: 2
            Layout.preferredWidth: guiSettings.threadColumnWidth
            Layout.fillHeight: true
            color: "transparent"

            Accessible.ignored: true

            Avatar {
                id: avatarImg
                x: parent.x + 8
                y: parent.y + 5
                width: parent.width - 13
                height: width
                author: page.author
            }
        }

        Column {
            Layout.fillWidth: true
            spacing: 3

            AuthorNameAndStatus {
                width: parent.width
                author: page.author
            }
            Text {
                width: parent.width
                elide: Text.ElideRight
                font.pointSize: guiSettings.scaledFont(7/8)
                color: guiSettings.handleColor
                text: `@${author.handle}`

                Accessible.ignored: true
            }
        }
    }

    AccessibleText {
        id: validityHeader
        anchors.top: authorGrid.bottom
        anchors.topMargin: 10
        width: parent.width
        font.bold: true
        text: qsTr('Duration:')
    }

    DurationInput {
        id: durationInput
        anchors.top: validityHeader.bottom
        width: parent.width
    }
}
