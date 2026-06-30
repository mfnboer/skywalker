import QtQuick
import QtQuick.Layouts
import skywalker


Rectangle {
    property string userDid
    property Skywalker skywalker: root.getSkywalker(userDid)
    property PostUtils postUtils: root.getPostUtils(userDid)
    required property int listType // QEnums::ChatAuthorListType
    required property convoview convo
    required property chatbasicprofile chatAuthor
    required property date joinRequestedAt
    readonly property basicprofile author: chatAuthor.basicProfile
    required property string followingUri
    required property string blockingUri
    required property bool authorMuted
    required property bool mutedReposts
    required property bool hideFromTimeline
    required property bool endOfList
    property bool allowDeleteItem: false
    property bool showAuthor: authorVisible()
    property bool showFollow: true
    property bool highlight: false
    property string highlightColor: guiSettings.postHighLightColor
    property int textRightPadding: 0
    readonly property int margin: 10

    signal follow(basicprofile profile)
    signal unfollow(string did, string uri)
    signal deleteItem(string did)
    signal clicked(basicprofile profile)

    id: authorRect
    height: grid.height
    color: highlight ? highlightColor : guiSettings.backgroundColor

    Accessible.role: Accessible.Button
    Accessible.name: author.name
    Accessible.onPressAction: skywalker.getDetailedProfile(author.did)

    GridLayout {
        id: grid
        columns: 3
        width: parent.width
        rowSpacing: 5
        columnSpacing: 10

        // Avatar
        Rectangle {
            id: avatar
            Layout.rowSpan: 4
            Layout.preferredWidth: guiSettings.threadColumnWidth
            Layout.fillHeight: true
            Layout.minimumHeight: guiSettings.threadColumnWidth
            color: "transparent"

            Accessible.ignored: true

            Avatar {
                id: avatarImg
                x: parent.x + 8
                y: parent.y + 5
                width: parent.width - 13
                userDid: authorRect.userDid
                author: authorRect.author
                onClicked: skywalker.getDetailedProfile(author.did)
            }
        }

        Column {
            Layout.fillWidth: true
            spacing: 3

            AuthorNameAndStatus {
                width: parent.width
                userDid: authorRect.userDid
                author: authorRect.author
            }
            AccessibleText {
                width: parent.width
                elide: Text.ElideRight
                font.pointSize: guiSettings.scaledFont(7/8)
                color: guiSettings.handleColor
                text: `@${author.handle}`

                Accessible.ignored: true
            }

            Item {
                width: parent.width
                height: viewerState.height > 0 ? 3 : 0
            }

            AuthorViewerState {
                id: viewerState
                userDid: authorRect.userDid
                did: author.did
                followedBy: author.viewer.followedBy && showFollow
                blockingUri: authorRect.blockingUri
                blockingByList: !author.viewer.blockingByList.isNull()
                blockedBy: author.viewer.blockedBy
                muted: authorMuted
                mutedByList: !author.viewer.mutedByList.isNull()
                mutedReposts: authorRect.mutedReposts
                hideFromTimeline: authorRect.hideFromTimeline
                convoMemberRole: chatAuthor.groupMember.role
            }

            ContentLabels {
                id: contentLabels
                anchors.left: parent.left
                anchors.right: undefined
                userDid: authorRect.userDid
                contentLabels: author.labels
                contentAuthor: author
            }
        }

        Row {
            Layout.fillHeight: true
            Layout.rightMargin: authorRect.margin

            SkyButton {
                height: 40
                text: qsTr("Follow")
                visible: !followingUri && author.did !== skywalker.getUserDid() && showAuthor && showFollow && !author.associated.isLabeler
                onClicked: follow(author)
                Accessible.name: qsTr(`press to follow ${author.name}`)
            }
            SvgButton {
                width: 40
                height: width
                svg: SvgOutline.delete
                accessibleName: qsTr(`press to remove ${author.name}`)
                visible: allowDeleteItem && chatAuthor.groupMember.role !== QEnums.CONVO_MEMBER_ROLE_OWNER
                onClicked: confirmDelete()
            }
        }

        SkyCleanedText {
            rightPadding: 10 + authorRect.textRightPadding
            Layout.columnSpan: 2
            Layout.fillWidth: true
            Layout.rightMargin: authorRect.margin
            inLayout: true
            width: parent.width
            elide: Text.ElideRight
            font.italic: true
            font.pointSize: guiSettings.scaledFont(7/8)
            plainText: author.pronouns
            visible: Boolean(author.pronouns)
        }

        SkyCleanedText {
            readonly property chatbasicprofile addedBy: chatAuthor.groupMember.addedBy

            rightPadding: 10 + authorRect.textRightPadding
            Layout.columnSpan: 2
            Layout.fillWidth: true
            Layout.rightMargin: authorRect.margin
            inLayout: true
            width: parent.width
            elide: Text.ElideRight
            font.pointSize: guiSettings.scaledFont(7/8)
            color: guiSettings.handleColor
            plainText: qsTr(`Added by ${addedBy.isNull() ? "invite link" : addedBy.basicProfile.name}`)
            visible: chatAuthor.groupMember.role !== QEnums.CONVO_MEMBER_ROLE_OWNER && listType === QEnums.CHAT_AUTHOR_LIST_MEMBERS
        }

        Loader {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            Layout.rightMargin: authorRect.margin
            active: listType === QEnums.CHAT_AUTHOR_LIST_JOIN_REQUESTS

            sourceComponent: Column {
                spacing: 10

                AccessibleText {
                    width: parent.width
                    elide: Text.ElideRight
                    font.pointSize: guiSettings.scaledFont(7/8)
                    text: qsTr(`Requested to join: ${guiSettings.dateTimeIndication(joinRequestedAt)}`)
                }

                RowLayout {
                    width: parent.width

                    SkyButton {
                        Layout.fillWidth: true
                        height: 40
                        text: qsTr("Approve")
                        onClicked: skywalker.chat.approveJoinRequest(convo.id, chatAuthor)
                    }
                    SkyButton {
                        Layout.fillWidth: true
                        height: 40
                        text: qsTr("Reject")
                        onClicked: confirmJoinRequestReject()
                    }
                }
            }
        }

        Rectangle {
            Layout.columnSpan: 3
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: authorRect.highlight ? guiSettings.separatorHighLightColor : guiSettings.separatorColor
        }

        // Create empty space (rowSpacing)
        Item {
            Layout.columnSpan: 3
            Layout.fillWidth: true
            height: 0
        }

        AccessibleText {
            Layout.columnSpan: 3
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            bottomPadding: 20
            elide: Text.ElideRight
            color: guiSettings.textColor
            text: qsTr("End of list")
            font.italic: true
            visible: endOfList
        }
    }
    SkyMouseArea {
        z: -1
        anchors.fill: parent
        onClicked: {
            authorRect.clicked(author)
            skywalker.getDetailedProfile(author.did)
        }
    }

    function confirmDelete() {
        guiSettings.askYesNoQuestion(
                    authorRect,
                    qsTr(`Do you really want to remove: @${author.handle} ?`),
                    () => deleteItem(author.did))
    }

    function confirmJoinRequestReject() {
        guiSettings.askYesNoQuestion(
                    authorRect,
                    qsTr(`Do you really want to reject: @${author.handle} ?`),
                    () => skywalker.chat.rejectJoinRequest(convo.id, chatAuthor))
    }

    function authorVisible() {
        return guiSettings.contentVisible(author, userDid)
    }
}
