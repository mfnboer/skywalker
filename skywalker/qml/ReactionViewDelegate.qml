import QtQuick
import QtQuick.Layouts
import skywalker

Rectangle {
    required property reactionview reaction
    required property convoview convo
    property basicprofile author: root.isActiveUser(reaction.senderDid) ? skywalker.user : convo.getMember(reaction.senderDid).basicProfile
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
                author: reactionRect.author
                onClicked: skywalker.getDetailedProfile(author.did)
            }
        }

        AuthorNameAndStatus {
            Layout.topMargin: rowPadding
            Layout.fillWidth: true
            author: reactionRect.author
        }

        Rectangle {
            Layout.rowSpan: 2
            Layout.preferredWidth: emojiText.width
            Layout.preferredHeight: emojiText.height
            color: "transparent"

            AccessibleText {
                id: emojiText
                font.pointSize: guiSettings.scaledFont(2)
                font.family: UnicodeFonts.getEmojiFontFamily()
                text: reaction.emoji
            }

            SkyMouseArea {
                anchors.fill: parent
                onClicked: skywalker.showStatusMessage(emojiNames.getEmojiName(reaction.emoji), QEnums.STATUS_LEVEL_INFO)
            }
        }

        AccessibleText {
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
                contentAuthor: author
            }
        }

        Item {
            visible: root.isActiveUser(reaction.senderDid)
        }

        AccessibleText {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            bottomPadding: rowPadding
            color: guiSettings.linkColor
            font.pointSize: guiSettings.scaledFont(7/8)
            text: qsTr("tap to delete")
            visible: root.isActiveUser(reaction.senderDid)
        }
    }
    SkyMouseArea {
        z: -1
        anchors.fill: parent
        enabled: root.isActiveUser(reaction.senderDid)
        onClicked: reactionRect.clicked()
    }

    EmojiNames {
        id: emojiNames
    }
}
