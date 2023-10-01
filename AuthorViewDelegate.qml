import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker


Rectangle {
    property int margin: 8
    required property int viewWidth

    required property profile author

    width: grid.width
    height: grid.height

    GridLayout {
        id: grid
        columns: 3
        width: viewWidth
        rowSpacing: 5
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
                text: author.handle
            }
        }

        Row {
            RoundButton {
                id: followButton
                Material.background: guiSettings.buttonColor
                contentItem: Text {
                    leftPadding: 10
                    rightPadding: 10
                    color: guiSettings.buttonTextColor
                    text: qsTr("Follow")
                }
                visible: !author.viewer.following && !isUser(author)

                //onClicked: graphUtils.follow(author.did)
            }
            RoundButton {
                id: unfollowButton
                flat: true
                Material.background: guiSettings.labelColor
                contentItem: Text {
                    leftPadding: 10
                    rightPadding: 10
                    color: guiSettings.textColor
                    text: qsTr("Following")
                }
                visible: author.viewer.following && !isUser(author)

                //onClicked: graphUtils.unfollow(following)
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

    GuiSettings {
        id: guiSettings
    }

    function isUser(author) {
        return skywalker.getUserDid() === author.did
    }
}
