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

    signal viewConvo(convoview convo)

    id: convoRect
    height: convoRow.height
    color: guiSettings.backgroundColor

    RowLayout {
        id: convoRow
        width: parent.width
        height: Math.max(avatarRect.height, convoColumn.height)
        spacing: 10

        Rectangle {
            id: avatarRect
            height: avatar.height + 10
            width: guiSettings.threadColumnWidth
            color: "transparent"

            Avatar {
                id: avatar
                x: parent.x + 8
                y: parent.y + 5
                width: parent.width - 13
                height: width
                avatarUrl: guiSettings.authorVisible(firstMember) ? firstMember.avatarUrl : ""
                isModerator: firstMember.associated.isLabeler
                onClicked: skywalker.getDetailedProfile(firstMember.did)

                BadgeCounter {
                    counter: convo.unreadCount
                }
            }
        }

        Column {
            id: convoColumn
            Layout.fillWidth: true
            spacing: 3

            RowLayout {
                width: parent.width
                spacing: 3

                SkyCleanedText {
                    Layout.fillWidth:  true
                    topPadding: 5
                    elide: Text.ElideRight
                    font.bold: true
                    color: guiSettings.textColor
                    plainText: convo.memberNames
                }

                AccessibleText {
                    Layout.rightMargin: margin
                    topPadding: 5
                    font.pointSize: guiSettings.scaledFont(6/8)
                    color: guiSettings.messageTimeColor
                    text: Qt.locale().toString(convo.lastMessageDate, Qt.locale().dateFormat(Locale.ShortFormat))
                }
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
                        avatarUrl: guiSettings.authorVisible(modelData.basicProfile) ? modelData.basicProfile.avatarUrl : ""
                        isModerator: modelData.basicProfile.associated.isLabeler
                        onClicked: viewConvo(convo)
                    }
                }

                visible: convo.members.length > 1
            }
        }
    }

    MouseArea {
        z: -2
        anchors.fill: parent
        onClicked: viewConvo(convo)
    }

    Rectangle {
        id: seperator
        anchors.top: convoRow.bottom
        width: parent.width
        height: 1
        color: guiSettings.separatorColor
    }

    // End of feed indication
    Text {
        anchors.top: seperator.bottom
        width: parent.width
        horizontalAlignment: Text.AlignHCenter
        topPadding: 10
        elide: Text.ElideRight
        color: guiSettings.textColor
        text: qsTr("End of conversations")
        font.italic: true
        visible: endOfList
    }

    GuiSettings {
        id: guiSettings
    }
}
