import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker


Rectangle {
    property int margin: 8
    required property int viewWidth
    required property profile author
    required property string followingUri
    property bool showAuthor: authorVisible()
    property bool showFollow: true

    signal follow(basicprofile profile)
    signal unfollow(string did, string uri)

    width: grid.width
    height: grid.height
    color: guiSettings.backgroundColor

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
                avatarUrl: showAuthor ? author.avatarUrl : ""
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
                color: guiSettings.textColor
                text: author.name
            }
            Text {
                width: parent.width
                elide: Text.ElideRight
                font.pointSize: guiSettings.scaledFont(7/8)
                color: guiSettings.handleColor
                text: `@${author.handle}`
            }

            ContentLabels {
                id: contentLabels
                anchors.left: parent.left
                anchors.right: undefined
                contentLabels: author.labels
            }

            Row {
                spacing: 5

                SkyLabel {
                    text: qsTr("follows you")
                    visible: author.viewer.followedBy && showFollow
                }
                SkyLabel {
                    text: qsTr("blocked")
                    visible: author.viewer.blocking
                }
                SkyLabel {
                    text: qsTr("muted")
                    visible: author.viewer.muted
                }
            }
        }

        Row {
            Layout.fillHeight: true

            SkyButton {
                text: qsTr("Follow")
                visible: !followingUri && !isUser(author) && showAuthor && showFollow
                onClicked: follow(author)
            }
            SkyButton {
                flat: true
                text: qsTr("Following")
                visible: followingUri && !isUser(author) && showAuthor && showFollow
                onClicked: unfollow(author.did, followingUri)
            }
        }

        Text {
            rightPadding: 10
            Layout.columnSpan: 2
            Layout.fillWidth: true
            wrapMode: Text.Wrap
            textFormat: Text.RichText
            color: guiSettings.textColor
            text: author.description
            visible: showAuthor
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
        onClicked: skywalker.getDetailedProfile(author.did)
    }

    GuiSettings {
        id: guiSettings
    }

    function authorVisible()
    {
        let visibility = skywalker.getContentVisibility(author.labels)
        return visibility === QEnums.CONTENT_VISIBILITY_SHOW
    }

    function isUser(author) {
        return skywalker.getUserDid() === author.did
    }
}