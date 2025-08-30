import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker


Rectangle {
    required property convoview convo
    required property bool endOfList
    property var skywalker: root.getSkywalker()
    property basicprofile firstMember: convo.members.length > 0 ? convo.members[0].basicProfile : skywalker.getUserProfile()
    readonly property list<contentlabel> labelsToShow: guiSettings.filterContentLabelsToShow(firstMember.labels)
    readonly property int margin: 10
    readonly property bool showLastReaction: !convo.lastReaction.isNull() && convo.lastReaction.reaction.createdAt > convo.lastMessageDate

    signal viewConvo(convoview convo)
    signal deleteConvo(convoview convo)
    signal muteConvo(convoview convo)
    signal unmuteConvo(convoview convo)
    signal blockAuthor(basicprofile author)
    signal unblockAuthor(basicprofile author)
    signal acceptConvo(convoview convo)
    signal blockAndDeleteConvo(convoview convo, basicprofile author)

    id: convoRect
    height: convoRow.height + (endOfFeedText.visible ? endOfFeedText.height : 0)
    color: guiSettings.backgroundColor

    RowLayout {
        id: convoRow
        y: 5
        width: parent.width
        height: Math.max(avatarRect.height, convoColumn.height)
        spacing: 10

        Rectangle {
            id: avatarRect
            Layout.alignment: Qt.AlignTop
            Layout.preferredHeight: avatar.height + 10
            Layout.preferredWidth: guiSettings.threadColumnWidth
            color: "transparent"

            Avatar {
                id: avatar
                x: parent.x + 8
                y: parent.y + 5
                width: parent.width - 13
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
                    color: guiSettings.textColor
                    plainText: convo.memberNames
                }

                SkySvg {
                    id: mutedImg
                    height: convo.muted ? nameText.height : 0
                    width: height
                    color: guiSettings.textColor
                    svg: SvgOutline.notificationsOff
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

            Row {
                width: parent.width - 10

                Column
                {
                    width: parent.width - moreButton.width

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
                                author: modelData.basicProfile
                                onClicked: viewConvo(convo)
                            }
                        }

                        visible: convo.members.length > 1
                    }

                    AuthorViewerState {
                        did: firstMember.did
                        blockingUri: firstMember.viewer.blocking
                        blockingByList: !firstMember.viewer.blockingByList.isNull()
                        blockedBy: firstMember.viewer.blockedBy
                        muted: firstMember.viewer.muted
                        mutedByList: !firstMember.viewer.mutedByList.isNull()
                    }

                    Loader {
                        active: convoRect.labelsToShow.length > 0 && convo.members.length <= 1
                        width: parent.width
                        height: active ? guiSettings.labelHeight + guiSettings.labelRowPadding : 0
                        asynchronous: true
                        visible: active

                        sourceComponent: Rectangle {
                            width: parent.width
                            height: parent.height
                            color: "transparent"

                            ContentLabels {
                                id: contentLabels
                                anchors.left: parent.left
                                anchors.right: undefined
                                contentLabels: firstMember.labels
                                labelsToShow: convoRect.labelsToShow
                                contentAuthorDid: firstMember.did
                            }
                        }
                    }

                    Loader {
                        width: parent.width
                        active: convo.status === QEnums.CONVO_STATUS_REQUEST
                        sourceComponent: KnownFollowers {
                            author: firstMember
                        }
                    }

                    // Last message
                    SkyCleanedText {
                        readonly property string deletedText: qsTr("message deleted")
                        readonly property bool sentByUser: convo.lastMessage.senderDid === skywalker.getUserDid()

                        width: parent.width
                        topPadding: 10
                        elide: Text.ElideRight
                        textFormat: Text.RichText
                        font.italic: convo.lastMessage.deleted
                        plainText: qsTr(`${(sentByUser ? "<i>You: </i>" : "")}${(!convo.lastMessage.deleted ? convo.lastMessage.text : deletedText)}`)
                        visible: !showLastReaction
                    }

                    // Last reaction
                    RowLayout {
                        width: parent.width
                        visible: showLastReaction

                        SkyCleanedText {
                            readonly property bool sentByUser: convo.lastReaction.reaction.senderDid === skywalker.getUserDid()

                            topPadding: 10
                            textFormat: Text.RichText
                            //plainText: qsTr(`${(sentByUser ? "<i>You: </i>" : "")}<span style="font-family:'${UnicodeFonts.getEmojiFontFamily()}'">${convo.lastReaction.reaction.emoji}</span> to: ${convo.lastReaction.message.text}`)
                            plainText: qsTr(`${(sentByUser ? "<i>You: </i>" : "")}<span style="font-family:'${UnicodeFonts.getEmojiFontFamily()}'">${convo.lastReaction.reaction.emoji}</span> to: `)
                        }
                        SkyCleanedText {
                            Layout.fillWidth: true
                            topPadding: 10
                            elide: Text.ElideRight
                            textFormat: Text.RichText
                            plainText: `${convo.lastReaction.message.text}`
                        }
                    }
                }

                SvgPlainButton {
                    id: moreButton
                    Layout.preferredWidth: 34
                    Layout.preferredHeight: 34
                    svg: SvgOutline.moreVert
                    accessibleName: qsTr("more options")
                    onClicked: moreMenu.open()

                    SkyMenu {
                        id: moreMenu
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
                        AccessibleMenuItem {
                            text: firstMember.viewer.blocking ? qsTr("Unblock account") : qsTr("Block account")
                            visible: !guiSettings.isUser(firstMember)
                            onTriggered: {
                                if (firstMember.viewer.blocking)
                                    unblockAuthor(firstMember)
                                else
                                    blockAuthor(firstMember)
                            }

                            MenuItemSvg { svg: firstMember.viewer.blocking ? SvgOutline.unblock : SvgOutline.block }
                        }
                    }
                }
            }

            Loader {
                width: parent.width - margin
                active: convo.status === QEnums.CONVO_STATUS_REQUEST
                sourceComponent: ConvoRequestButtonRow {
                    author: firstMember

                    onAcceptConvo: convoRect.acceptConvo(convo)
                    onDeleteConvo: convoRect.deleteConvo(convo)
                    onBlockAndDeleteConvo: convoRect.blockAndDeleteConvo(convo, firstMember)
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
        id: endOfFeedText
        anchors.top: convoRow.bottom
        width: parent.width
        horizontalAlignment: Text.AlignHCenter
        topPadding: 30
        bottomPadding: 100
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
}
