import QtQuick
import QtQuick.Layouts
import skywalker

Rectangle {
    required property reactionview reaction
    required property convoview convo
    property basicprofile author: guiSettings.isUserDid(reaction.senderDid) ? skywalker.user : convo.getMember(reaction.senderDid).basicProfile
    property var skywalker: root.getSkywalker()
    property int rowPadding: 3

    signal clicked

    id: reactionRect
    height: grid.height
    color: guiSettings.backgroundColor

    GridLayout {
        id: grid
        columns: 3
        x: 5
        width: parent.width - 15
        rowSpacing: 0
        columnSpacing: 10

        // Avatar
        Rectangle {
            id: avatar
            Layout.rowSpan: 2
            Layout.preferredWidth: 44
            Layout.preferredHeight: avatarImg.height + rowPadding + 2
            Layout.fillHeight: true
            color: "transparent"

            Accessible.ignored: true

            Avatar {
                id: avatarImg
                x: 8
                y: rowPadding + 2
                width: parent.width - 12
                height: width
                author: reactionRect.author
                onClicked: skywalker.getDetailedProfile(author.did)
            }
        }

        SkyCleanedTextLine {
            topPadding: rowPadding
            Layout.fillWidth: true
            elide: Text.ElideRight
            font.bold: true
            color: guiSettings.textColor
            plainText: author.name

            Accessible.ignored: true
        }

        Rectangle {
            Layout.rowSpan: 2
            Layout.preferredWidth: emojiText.width
            Layout.preferredHeight: emojiText.height
            color: "transparent"

            AccessibleText {
                id: emojiText
                font.pointSize: guiSettings.scaledFont(2)
                text: reaction.emoji
            }
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
            Layout.preferredHeight: contentLabels.height + 5
            color: "transparent"

            ContentLabels {
                id: contentLabels
                anchors.left: parent.left
                anchors.right: undefined
                contentLabels: author.labels
                contentAuthorDid: author.did
            }
        }

        Item {
            visible: guiSettings.isUserDid(reaction.senderDid)
        }

        AccessibleText {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            bottomPadding: rowPadding
            color: guiSettings.linkColor
            font.pointSize: guiSettings.scaledFont(7/8)
            text: qsTr("tap to delete")
            visible: guiSettings.isUserDid(reaction.senderDid)
        }
    }
    MouseArea {
        z: -1
        anchors.fill: parent
        enabled: guiSettings.isUserDid(reaction.senderDid)
        onClicked: reactionRect.clicked()
    }
}
