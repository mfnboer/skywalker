import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker


Rectangle {
    property int margin: 8
    required property int viewWidth
    required property profile author
    required property string followingUri

    signal follow(basicprofile profile)
    signal unfollow(string did, string uri)

    width: grid.width
    height: grid.height

    GridLayout {
        id: grid
        columns: 3
        width: viewWidth
        rowSpacing: 10
        columnSpacing: 10

        // Avatar
        Rectangle {
            id: avatar
            Layout.rowSpan: 2
            width: guiSettings.threadBarWidth * 5
            Layout.fillHeight: true
            color: "transparent"

            Avatar {
                id: avatarImg
                x: parent.x + 8
                y: parent.y + 5
                width: parent.width - 13
                height: width
                avatarUrl: author.avatarUrl
                onClicked: skywalker.getDetailedProfile(author.did)
            }
        }

        Column {
            Layout.fillWidth: true
            spacing: 3

            Text {
                width: parent.width
                elide: Text.ElideRight
                font.bold: true
                text: author.name
            }
            Text {
                width: parent.width
                elide: Text.ElideRight
                font.pointSize: guiSettings.scaledFont(7/8)
                color: guiSettings.handleColor
                text: `@${author.handle}`
            }
            SkyLabel {
                text: qsTr("follows you")
                visible: author.viewer.followedBy
            }
        }

        Row {
            Layout.fillHeight: true

            SkyButton {
                text: qsTr("Follow")
                visible: !followingUri && !isUser(author)
                onClicked: follow(author)
            }
            SkyButton {
                flat: true
                text: qsTr("Following")
                visible: followingUri && !isUser(author)
                onClicked: unfollow(author.did, followingUri)
            }
        }

        Text {
            rightPadding: 10
            Layout.columnSpan: 2
            Layout.fillWidth: true
            wrapMode: Text.Wrap
            textFormat: Text.RichText
            text: author.description
        }

        Rectangle {
            Layout.columnSpan: 3
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: "lightgrey"
        }
    }
    MouseArea {
        z: -1
        anchors.fill: parent
        onClicked: skywalker.getDetailedProfile(author.did)
    }

    GuiSettings {
        id: guiSettings
    }

    function isUser(author) {
        return skywalker.getUserDid() === author.did
    }
}
