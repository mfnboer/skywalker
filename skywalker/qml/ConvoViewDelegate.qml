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
    readonly property bool userIsOwner: convo.getMember(skywalker.getUserDid()).groupMember.role === QEnums.CONVO_MEMBER_ROLE_OWNER
    readonly property list<contentlabel> labelsToShow: guiSettings.filterContentLabelsToShow(firstMember, firstMember.labels)
    readonly property int margin: 10
    readonly property bool showLastReaction: !convo.lastReaction.isNull() && convo.lastReaction.reaction.createdAt > convo.lastMessageDate

    signal viewConvo(convoview convo)
    signal deleteConvo(convoview convo)
    signal leaveConvo(convoview convo)
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

                    ConvoMembersRow {
                        width: parent.width
                        convo: convoRect.convo
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
                        active: convo.status === QEnums.CONVO_STATUS_REQUEST && !convo.isRequestToJoin
                        sourceComponent: KnownFollowers {
                            userDid: skywalker.getUserDid()
                            author: firstMember
                        }
                    }

                    Loader {
                        width: parent.width
                        active: convo.isRequestToJoin
                        sourceComponent: Column {
                            spacing: 5

                            AccessibleText {
                                width: parent.width
                                elide: Text.ElideRight
                                font.pointSize: guiSettings.scaledFont(7/8)
                                text: qsTr(`Group chat ${convo.group.memberCount}/${convo.group.memberLimit} members`)
                            }
                            AccessibleText {
                                width: parent.width
                                elide: Text.ElideRight
                                font.italic: true
                                text: qsTr("You requested to join")
                            }
                        }
                    }

                    // Last message
                    SkyCleanedText {
                        property string senderName: showLastReaction ? "" : getLastMessageSender()
                        property string messageText: getLastMessageText(convo.lastMessage)

                        id: lastMessage
                        width: parent.width
                        topPadding: 10
                        elide: Text.ElideRight
                        wrapMode: Text.Wrap
                        maximumLineCount: 2
                        textFormat: Text.StyledText
                        font.italic: convo.lastMessage.deleted || convo.lastMessage.isSystemMessage
                        plainText: (senderName ? `<i>${senderName}: </i>` : "") + messageText
                        visible: !showLastReaction && !convo.group.isLocked() && !convo.isRequestToJoin
                    }

                    // Last reaction
                    SkyCleanedText {
                        property string senderName: showLastReaction ? getLastReactionSender() : ""
                        property string messageText: getLastMessageText(convo.lastReaction.message)

                        id: lastReaction
                        width: parent.width
                        topPadding: 10
                        elide: Text.ElideRight
                        wrapMode: Text.Wrap
                        maximumLineCount: 2
                        textFormat: Text.StyledText
                        inLayout: true
                        plainText: (senderName ? `<i>${senderName} </i>` : "") +
                                   qsTr(`reacted <span style="font-family:'${UnicodeFonts.getEmojiFontFamily()}'">${convo.lastReaction.reaction.emoji}</span> to: `) +
                                   `${messageText}`
                        visible: showLastReaction && !convo.group.isLocked() && !convo.isRequestToJoin
                    }

                    // Join requests to approve
                    Loader {
                        width: parent.width
                        active: convo.group.joinRequestCount > 0

                        sourceComponent: AccessibleText {
                            topPadding: 10
                            wrapMode: Text.Wrap
                            font.bold: convo.group.unreadJoinRequestCount > 0
                            color: guiSettings.accentColor
                            text: convo.group.joinRequestCount === 1 ?
                                      qsTr("1 join request pending") :
                                      qsTr(`${convo.group.joinRequestCount} join requests pending`)

                            MouseArea {
                                anchors.fill: parent
                                onClicked: root.viewConvoAuthorList(QEnums.CHAT_AUTHOR_LIST_JOIN_REQUESTS, convo, skywalker.getUserDid())
                            }
                        }
                    }

                    // Locked
                    Loader {
                        width: parent.width
                        active: convo.group.isLocked()

                        sourceComponent: Row {
                            SkySvg {
                                id: lockImg
                                y: height + 10
                                width: guiSettings.appFontHeight
                                height: width
                                svg: SvgOutline.lock
                            }

                            AccessibleText {
                                topPadding: 10
                                width: parent.width - lockImg.width
                                anchors.verticalCenter: parent.verticalCenter
                                font.italic: true
                                elide: Text.ElideRight
                                text: guiSettings.getChatLockedText(convo)
                            }
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
                            text: qsTr("Join requests")
                            svg: SvgOutline.enter
                            popup: moreMenu
                            visible: convo.group.joinRequestCount > 0
                            onClicked: root.viewConvoAuthorList(QEnums.CHAT_AUTHOR_LIST_JOIN_REQUESTS, convo, skywalker.getUserDid())
                        }
                        SkyMenuButton {
                            text: qsTr("Members")
                            svg: SvgOutline.group
                            popup: moreMenu
                            visible: convo.kind === QEnums.CONVO_KIND_GROUP && !convo.isRequestToJoin
                            onClicked: root.viewConvoAuthorList(QEnums.CHAT_AUTHOR_LIST_MEMBERS, convo, skywalker.getUserDid())
                        }
                        SkyMenuButton {
                            text: qsTr("Invite link")
                            svg: SvgOutline.link
                            popup: moreMenu
                            visible: convo.kind === QEnums.CONVO_KIND_GROUP && !convo.isRequestToJoin &&
                                     (userIsOwner || convo.group.joinLinkView.enabledStatus === QEnums.JOIN_LINK_ENABLED_STATUS_ENABLED)

                            onClicked: {
                                if (convo.group.joinLinkView.isNull())
                                    root.createOrEditJoinLink(convo)
                                else
                                    root.showJoinLink(convo)
                            }
                        }
                        SkyMenuButton {
                            text: qsTr("Edit group name")
                            svg: SvgOutline.edit
                            popup: moreMenu
                            visible: convo.kind === QEnums.CONVO_KIND_GROUP && userIsOwner
                            onClicked: root.editGroupName(convo)
                        }
                        SkyMenuButton {
                            text: qsTr("Lock")
                            svg: SvgOutline.lock
                            popup: moreMenu
                            visible: convo.kind === QEnums.CONVO_KIND_GROUP && convo.group.lockStatus === QEnums.CONVO_LOCK_STATUS_UNLOCKED && userIsOwner
                            onClicked: root.lockGroupConvo(convo.id)
                        }
                        SkyMenuButton {
                            text: qsTr("unlock")
                            svg: SvgOutline.lockOpen
                            popup: moreMenu
                            visible: convo.kind === QEnums.CONVO_KIND_GROUP && convo.group.lockStatus === QEnums.CONVO_LOCK_STATUS_LOCKED && userIsOwner
                            onClicked: skywalker.chat.unlockGroupConvo(convo.id)
                        }
                        SkyMenuButton {
                            text: qsTr("Delete")
                            svg: SvgOutline.delete
                            popup: moreMenu
                            visible: convo.kind !== QEnums.CONVO_KIND_GROUP
                            onClicked: deleteConvo(convo)
                        }
                        SkyMenuButton {
                            text: qsTr("Leave")
                            svg: SvgOutline.signOut
                            popup: moreMenu
                            visible: convo.kind === QEnums.CONVO_KIND_GROUP && !convo.isRequestToJoin
                            onClicked: leaveConvo(convo)
                        }
                        SkyMenuButton {
                            text: convo.muted ? qsTr("Unmute") : qsTr("Mute")
                            svg: convo.muted ? SvgOutline.notifications : SvgOutline.notificationsOff
                            popup: moreMenu
                            visible: !convo.isRequestToJoin
                            onClicked: convo.muted ? unmuteConvo(convo) : muteConvo(convo)
                        }
                        SkyMenuButton {
                            text: firstMember.viewer.blocking ? qsTr("Unblock account") : qsTr("Block account")
                            svg: firstMember.viewer.blocking ? SvgOutline.unblock : SvgOutline.block
                            popup: moreMenu
                            visible: !root.isActiveUser(firstMember.did) && !convo.isRequestToJoin
                            onClicked: {
                                if (firstMember.viewer.blocking)
                                    unblockAuthor(firstMember)
                                else
                                    blockAuthor(firstMember)
                            }
                        }
                        SkyMenuButton {
                            text: qsTr("Cancel request")
                            svg: SvgOutline.cancel
                            popup: moreMenu
                            visible: convo.isRequestToJoin
                            // TODO
                        }
                    }
                }
            }

            Loader {
                width: parent.width - margin
                active: convo.status === QEnums.CONVO_STATUS_REQUEST && !convo.isRequestToJoin
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
        enabled: !convo.isRequestToJoin
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
        text: qsTr("End of chats")
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
        if (convo.isRequestToJoin)
            return getTimeIndication(convo.joinRequestedAt)

        return getTimeIndication(convo.lastMessageDate)
    }

    function getTimeIndication(date) {
        if (guiSettings.isToday(date))
            return Qt.locale().toString(date, Qt.locale().timeFormat(Locale.ShortFormat))
        else if (guiSettings.isYesterday(date))
            return qsTr("Yesterday")
        else
            return Qt.locale().toString(date, Qt.locale().dateFormat(Locale.ShortFormat))
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

    function getLastMessageText(lastMessage) {
        if (lastMessage.isNull())
            return qsTr("(no messages)")

        if (lastMessage.isSystemMessage) {
            Qt.callLater(() => { systemMessageUtils.active = true })
            return ""
        }

        return guiSettings.getChatMessageQuoteText(lastMessage, true)
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
