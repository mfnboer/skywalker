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
    signal deleteConvo(convoview convo)
    signal muteConvo(convoview convo)
    signal unmuteConvo(convoview convo)

    id: convoRect
    height: convoRow.height
    color: guiSettings.backgroundColor

    RowLayout {
        id: convoRow
        y: 5
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
                    counter: convo.muted ? 0 : convo.unreadCount
                }
            }
        }

        Column {
            id: convoColumn
            Layout.fillWidth: true
            spacing: 0

            Row {
                width: parent.width
                spacing: 3

                SkyCleanedText {
                    id: nameText
                    width: parent.width - mutedImg.width - timeText.width - parent.spacing * (convo.muted ? 2 : 1)
                    elide: Text.ElideRight
                    font.bold: true
                    color: guiSettings.textColor
                    plainText: convo.memberNames
                }

                SvgImage {
                    id: mutedImg
                    height: convo.muted ? nameText.height : 0
                    width: height
                    color: guiSettings.textColor
                    svg: svgOutline.notificationsOff
                    visible: convo.muted
                }

                AccessibleText {
                    id: timeText
                    rightPadding: margin
                    font.pointSize: guiSettings.scaledFont(6/8)
                    color: guiSettings.messageTimeColor
                    text: getConvoTimeIndication()
                }
            }

            RowLayout {
                width: parent.width - 10

                Column
                {
                    Layout.fillWidth: true

                    AccessibleText {
                        width: parent.width
                        elide: Text.ElideRight
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

                    SkyCleanedText {
                        readonly property string deletedText: qsTr("message deleted")
                        readonly property bool sentByUser: convo.lastMessage.senderDid === skywalker.getUserDid()

                        width: parent.width
                        topPadding: 5
                        elide: Text.ElideRight
                        textFormat: Text.RichText
                        font.italic: convo.lastMessage.deleted
                        plainText: qsTr(`${(sentByUser ? "<i>You: </i>" : "")}${(!convo.lastMessage.deleted ? convo.lastMessage.formattedText : deletedText)}`)
                    }
                }

                SvgButton {
                    id: moreButton
                    Layout.preferredWidth: 34
                    Layout.preferredHeight: 34
                    svg: svgOutline.moreVert
                    accessibleName: qsTr("more options")
                    onClicked: moreMenu.open()

                    Menu {
                        id: moreMenu
                        modal: true

                        onAboutToShow: root.enablePopupShield(true)
                        onAboutToHide: root.enablePopupShield(false)

                        CloseMenuItem {
                            text: qsTr("<b>Conversation</b>")
                            Accessible.name: qsTr("close conversations menu")
                        }
                        AccessibleMenuItem {
                            text: qsTr("Delete")
                            onTriggered: deleteConvo(convo)

                            MenuItemSvg { svg: svgOutline.delete }
                        }
                        AccessibleMenuItem {
                            text: convo.muted ? qsTr("Unmute") : qsTr("Mute")
                            onTriggered: convo.muted ? unmuteConvo(convo) : muteConvo(convo)

                            MenuItemSvg { svg: convo.muted ? svgOutline.notifications : svgOutline.notificationsOff }
                        }
                    }
                }
            }
        }
    }

    MouseArea {
        z: -2
        anchors.fill: parent
        onClicked: viewConvo(convo)
    }

    // End of feed indication
    Text {
        anchors.top: convoRow.bottom
        width: parent.width
        horizontalAlignment: Text.AlignHCenter
        topPadding: 10
        elide: Text.ElideRight
        color: guiSettings.textColor
        text: qsTr("End of conversations")
        font.italic: true
        visible: endOfList
    }

    function getConvoTimeIndication() {
        if (guiSettings.isToday(convo.lastMessageDate))
            return Qt.locale().toString(convo.lastMessageDate, Qt.locale().timeFormat(Locale.ShortFormat))
        else if (guiSettings.isYesterday(convo.lastMessageDate))
            return qsTr("Yesterday")
        else
            return Qt.locale().toString(convo.lastMessageDate, Qt.locale().dateFormat(Locale.ShortFormat))
    }

    GuiSettings {
        id: guiSettings
    }
}
