import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker


Rectangle {
    required property convoview convo
    required property bool endOfList
    property Skywalker skywalker: root.getSkywalker()
    property basicprofile firstMember: convo.members.length > 0 ? convo.members[0].basicProfile : skywalker.getUserProfile()
    readonly property list<contentlabel> labelsToShow: guiSettings.filterContentLabelsToShow(firstMember, firstMember.labels)
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
                showGroupIcon: convo.kind === QEnums.CONVO_KIND_GROUP
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
                    plainText: convo.title
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
                            id: activeMembers
                            model: convo.members.slice(1, 6) // Show 5 max

                            Avatar {
                                required property chatbasicprofile modelData

                                width: guiSettings.avatarSmallWidth
                                author: modelData.basicProfile
                                showFollowingStatus: false
                                onClicked: viewConvo(convo)
                            }
                        }
                        AccessibleText {
                            anchors.verticalCenter: parent.verticalCenter
                            text: `+${(convo.group.memberCount - activeMembers.count - 1 )}`
                            visible: convo.group.memberCount > activeMembers.count + 1
                        }

                        visible: convo.members.length > 1
                    }

                    Item {
                        width: parent.width
                        height: viewerState.height > 0 ? 3 : 0
                    }

                    AuthorViewerState {
                        id: viewerState
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
                        visible: active

                        sourceComponent: Rectangle {
                            width: parent.width
                            height: contentLabels.height + guiSettings.labelRowPadding
                            color: "transparent"

                            ContentLabels {
                                id: contentLabels
                                anchors.left: parent.left
                                anchors.right: undefined
                                contentLabels: firstMember.labels
                                labelsToShow: convoRect.labelsToShow
                                contentAuthor: firstMember
                            }
                        }
                    }

                    Loader {
                        width: parent.width
                        active: convo.status === QEnums.CONVO_STATUS_REQUEST
                        sourceComponent: KnownFollowers {
                            userDid: skywalker.getUserDid()
                            author: firstMember
                        }
                    }

                    // TODO: loaders
                    // Last message
                    SkyCleanedText {
                        property string senderName: showLastReaction ? "" : getLastMessageSender()
                        property string messageText: getLastMessageText()

                        id: lastMessage
                        width: parent.width
                        topPadding: 10
                        elide: Text.ElideRight
                        textFormat: Text.RichText
                        font.italic: convo.lastMessage.deleted || convo.lastMessage.isSystemMessage
                        plainText: (senderName ? `<i>${senderName}: </i>` : "") + messageText
                        visible: !showLastReaction
                    }

                    // Last reaction
                    RowLayout {
                        width: parent.width
                        visible: showLastReaction

                        SkyCleanedText {
                            property string senderName: showLastReaction ? getLastReactionSender() : ""

                            id: lastReaction
                            topPadding: 10
                            textFormat: Text.RichText
                            inLayout: true
                            plainText: (senderName ? `<i>${senderName} - </i>` : "") + qsTr(`<span style="font-family:'${UnicodeFonts.getEmojiFontFamily()}'">${convo.lastReaction.reaction.emoji}</span> to: `)
                        }
                        SkyCleanedText {
                            Layout.fillWidth: true
                            topPadding: 10
                            elide: Text.ElideRight
                            textFormat: Text.RichText
                            inLayout: true
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

                        SkyMenuButton {
                            text: qsTr("Delete")
                            svg: SvgOutline.delete
                            popup: moreMenu
                            onClicked: deleteConvo(convo)
                        }
                        SkyMenuButton {
                            text: convo.muted ? qsTr("Unmute") : qsTr("Mute")
                            svg: convo.muted ? SvgOutline.notifications : SvgOutline.notificationsOff
                            popup: moreMenu
                            onClicked: convo.muted ? unmuteConvo(convo) : muteConvo(convo)
                        }
                        SkyMenuButton {
                            text: firstMember.viewer.blocking ? qsTr("Unblock account") : qsTr("Block account")
                            svg: firstMember.viewer.blocking ? SvgOutline.unblock : SvgOutline.block
                            popup: moreMenu
                            visible: !root.isActiveUser(firstMember.did)
                            onClicked: {
                                if (firstMember.viewer.blocking)
                                    unblockAuthor(firstMember)
                                else
                                    blockAuthor(firstMember)
                            }
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

    SkyMouseArea {
        z: -2
        anchors.fill: parent
        onClicked: viewConvo(convo)
    }

    // End of feed indication
    AccessibleText {
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

    Loader {
        property bool lastReactionSender: false

        id: profileLoader
        active: false

        sourceComponent: ProfileUtils {
            skywalker: convoRect.skywalker

            onBasicProfileOk: (profile) => {
                if (profileLoader.lastReactionSender)
                    lastReaction.senderName = profile.name
                else
                    lastMessage.senderName = profile.name
            }
        }

        onStatusChanged: {
            if (status != Loader.Ready)
                return

            if (lastReactionSender)
                item.getBasicProfile(convo.lastReaction.reaction.senderDid)
            else
                item.getBasicProfile(convo.lastMessage.senderDid)
        }

        function getLastMessageSender() {
            lastReactionSender = false
            active = true
        }

        function getLastReactionSender() {
            lastReactionSender = true
            active = true
        }
    }

    Loader {
        id: systemMessageUtils
        active: false

        sourceComponent: SystemMessageUtils {
            onMessage: (icon, text) => lastMessage.messageText = text
        }

        onStatusChanged: {
            if (status == Loader.Ready)
                item.getMessage(convo.lastMessage)
        }
    }

    function getConvoTimeIndication() {
        if (guiSettings.isToday(convo.lastMessageDate))
            return Qt.locale().toString(convo.lastMessageDate, Qt.locale().timeFormat(Locale.ShortFormat))
        else if (guiSettings.isYesterday(convo.lastMessageDate))
            return qsTr("Yesterday")
        else
            return Qt.locale().toString(convo.lastMessageDate, Qt.locale().dateFormat(Locale.ShortFormat))
    }

    function getLastMessageSender() {
        if (convo.lastMessage.isNull())
            return ""

        if (convo.lastMessage.isSystemMessage)
            return ""

        if (convo.lastMessage.senderDid === skywalker.getUserDid())
            return qsTr("You");

        if (convo.kind === QEnums.CONVO_KIND_GROUP) {
            Qt.callLater(() => { profileLoader.getLastMessageSender() })
            return ""
        }

        return "";
    }

    function getLastMessageText() {
        if (convo.lastMessage.isNull())
            return ""

        if (convo.lastMessage.isSystemMessage) {
            Qt.callLater(() => { systemMessageUtils.active = true })
            return ""
        }

        if (convo.lastMessage.deleted)
            return qsTr("message deleted")

        return convo.lastMessage.text
    }

    function getLastReactionSender() {
        if (convo.lastReaction.reaction.isNull())
            return ""

        if (convo.lastReaction.reaction.senderDid === skywalker.getUserDid())
            return qsTr("You")

        if (convo.kind === QEnums.CONVO_KIND_GROUP) {
            Qt.callLater(() => { profileLoader.getLastReactionSender() })
            return ""
        }

        return "";
    }
}
