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
    color: GuiSettings.backgroundColor

    RowLayout {
        id: convoRow
        y: 5
        width: parent.width
        height: Math.max(avatarRect.height, convoColumn.height)
        spacing: 10

        Rectangle {
            id: avatarRect
            height: avatar.height + 10
            width: GuiSettings.threadColumnWidth
            color: "transparent"

            Avatar {
                id: avatar
                x: parent.x + 8
                y: parent.y + 5
                width: parent.width - 13
                height: width
                author: firstMember
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

                SkyCleanedTextLine {
                    id: nameText
                    width: parent.width - mutedImg.width - timeText.width - parent.spacing * (convo.muted ? 2 : 1)
                    elide: Text.ElideRight
                    font.bold: true
                    color: GuiSettings.textColor
                    plainText: convo.memberNames
                }

                SkySvg {
                    id: mutedImg
                    height: convo.muted ? nameText.height : 0
                    width: height
                    color: GuiSettings.textColor
                    svg: SvgOutline.notificationsOff
                    visible: convo.muted
                }

                AccessibleText {
                    id: timeText
                    rightPadding: margin
                    font.pointSize: GuiSettings.scaledFont(6/8)
                    color: GuiSettings.messageTimeColor
                    text: getConvoTimeIndication()
                }
            }

            Row {
                width: parent.width - 10

                Column
                {
                    width: parent.width - moreButton.width

                    AccessibleText {
                        width: parent.width
                        elide: Text.ElideRight
                        color: GuiSettings.handleColor
                        font.pointSize: GuiSettings.scaledFont(7/8)
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
                                author: modelData.basicProfile
                                onClicked: viewConvo(convo)
                            }
                        }

                        visible: convo.members.length > 1
                    }

                    SkyCleanedTextLine {
                        readonly property string deletedText: qsTr("message deleted")
                        readonly property bool sentByUser: convo.lastMessage.senderDid === skywalker.getUserDid()

                        width: parent.width
                        topPadding: 5
                        elide: Text.ElideRight
                        textFormat: Text.RichText
                        font.italic: convo.lastMessage.deleted
                        plainText: qsTr(`${(sentByUser ? "<i>You: </i>" : "")}${(!convo.lastMessage.deleted ? convo.lastMessage.text : deletedText)}`)
                    }
                }

                SvgButton {
                    id: moreButton
                    Layout.preferredWidth: 34
                    Layout.preferredHeight: 34
                    iconColor: GuiSettings.textColor
                    Material.background: "transparent"
                    svg: SvgOutline.moreVert
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

                            MenuItemSvg { svg: SvgOutline.delete }
                        }
                        AccessibleMenuItem {
                            text: convo.muted ? qsTr("Unmute") : qsTr("Mute")
                            onTriggered: convo.muted ? unmuteConvo(convo) : muteConvo(convo)

                            MenuItemSvg { svg: convo.muted ? SvgOutline.notifications : SvgOutline.notificationsOff }
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
        color: GuiSettings.textColor
        text: qsTr("End of conversations")
        font.italic: true
        visible: endOfList
    }

    function getConvoTimeIndication() {
        if (GuiSettings.isToday(convo.lastMessageDate))
            return Qt.locale().toString(convo.lastMessageDate, Qt.locale().timeFormat(Locale.ShortFormat))
        else if (GuiSettings.isYesterday(convo.lastMessageDate))
            return qsTr("Yesterday")
        else
            return Qt.locale().toString(convo.lastMessageDate, Qt.locale().dateFormat(Locale.ShortFormat))
    }

}
