import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker


Rectangle {
    required property convoview convo
    required property bool endOfList
    property var skywalker: root.getSkywalker()
    property basicprofile firstMember: convo.members.length > 0 ? convo.members[0].basicProfile : skywalker.getUserProfile()
    readonly property int margin: 10

    id: convoRect
    height: convoRow.height
    color: guiSettings.backgroundColor

    RowLayout {
        id: convoRow
        width: parent.width
        spacing: 10

        Rectangle {
            Layout.fillHeight: true
            width: guiSettings.threadColumnWidth
            color: "transparent"

            Avatar {
                x: parent.x + 8
                y: parent.y + 5
                width: parent.width - 13
                height: width
                avatarUrl: authorVisible(firstMember) ? firstMember.avatarUrl : ""
                isModerator: firstMember.associated.isLabeler
                onClicked: skywalker.getDetailedProfile(firstMember.did)

                BadgeCounter {
                    counter: convo.unreadCount
                }
            }
        }

        Column {
            Layout.fillWidth: true
            Layout.rightMargin: margin
            spacing: 3

            SkyCleanedText {
                width: parent.width
                Layout.fillHeight: true
                topPadding: 5
                elide: Text.ElideRight
                font.bold: true
                color: guiSettings.textColor
                plainText: convo.memberNames
            }

            AccessibleText {
                width: parent.width
                Layout.preferredHeight: visible ? implicitHeight : 0
                color: guiSettings.handleColor
                font.pointSize: guiSettings.scaledFont(7/8)
                text: `@${firstMember.handle}`
                visible: convo.members.length <= 1
            }

            Row {
                width: parent.width
                spacing: 3

                Repeater {
                    model: convo.members.slice(1)

                    Avatar {
                        required property chatbasicprofile modelData

                        width: 25
                        height: width
                        avatarUrl: authorVisible(modelData.basicProfile) ? modelData.basicProfile.avatarUrl : ""
                        isModerator: modelData.basicProfile.associated.isLabeler
                    }
                }

                visible: convo.members.length > 1
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: guiSettings.separatorColor
        }
    }

    GuiSettings {
        id: guiSettings
    }

    function authorVisible(author)
    {
        let visibility = skywalker.getContentVisibility(author.labels)
        return visibility === QEnums.CONTENT_VISIBILITY_SHOW
    }
}
