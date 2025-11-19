import QtQuick
import QtQuick.Layouts
import skywalker


Rectangle {
    property string userDid
    property Skywalker skywalker: root.getSkywalker(userDid)
    property PostUtils postUtils: root.getPostUtils(userDid)
    required property profile author
    required property string followingUri
    required property string blockingUri
    required property activitysubscription activitySubscription
    required property string listItemUri // empty when the author list is not an item list
    required property bool authorMuted
    required property bool mutedReposts
    required property bool hideFromTimeline
    required property bool endOfList
    property bool allowDeleteItem: false
    property bool showAuthor: authorVisible()
    property bool showFollow: true
    property bool showActivitySubscription: false
    property bool highlight: false
    property string highlightColor: guiSettings.postHighLightColor
    property int maximumDescriptionLineCount: 25
    property int textRightPadding: 0
    readonly property int margin: 10

    signal follow(basicprofile profile)
    signal unfollow(string did, string uri)
    signal deleteItem(string listItemUri)
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
        rowSpacing: 10
        columnSpacing: 10

        // Avatar
        Rectangle {
            id: avatar
            Layout.rowSpan: 3
            Layout.preferredWidth: guiSettings.threadColumnWidth
            Layout.fillHeight: true
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
            Text {
                width: parent.width
                elide: Text.ElideRight
                font.pointSize: guiSettings.scaledFont(7/8)
                color: guiSettings.handleColor
                text: `@${author.handle}`

                Accessible.ignored: true
            }

            AuthorViewerState {
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
                showActivitySubscription: authorRect.showActivitySubscription
                activitySubscription: authorRect.activitySubscription
            }

            ContentLabels {
                id: contentLabels
                anchors.left: parent.left
                anchors.right: undefined
                userDid: authorRect.userDid
                contentLabels: author.labels
                contentAuthorDid: author.did
            }
        }

        Row {
            Layout.fillHeight: true
            Layout.rightMargin: authorRect.margin

            SkyButton {
                text: qsTr("Follow")
                visible: !followingUri && author.did !== skywalker.getUserDid() && showAuthor && showFollow && !author.associated.isLabeler
                onClicked: follow(author)
                Accessible.name: qsTr(`press to follow ${author.name}`)
            }
            SkyButton {
                flat: true
                text: qsTr("Following")
                visible: followingUri && author.did !== skywalker.getUserDid() && showAuthor && showFollow && !author.associated.isLabeler
                onClicked: unfollow(author.did, followingUri)
                Accessible.name: qsTr(`press to unfollow ${author.name}`)
            }
            SvgButton {
                svg: SvgOutline.delete
                accessibleName: qsTr(`press to delete ${author.name}`)
                visible: listItemUri && allowDeleteItem
                onClicked: confirmDelete()
            }
        }

        SkyCleanedText {
            rightPadding: 10 + authorRect.textRightPadding
            Layout.columnSpan: 2
            Layout.fillWidth: true
            Layout.rightMargin: authorRect.margin
            width: parent.width
            font.italic: true
            font.pointSize: guiSettings.scaledFont(7/8)
            plainText: author.pronouns
            visible: Boolean(author.pronouns)
        }

        SkyCleanedText {
            rightPadding: 10 + authorRect.textRightPadding
            Layout.columnSpan: 2
            Layout.fillWidth: true
            Layout.rightMargin: authorRect.margin
            wrapMode: Text.Wrap
            elide: Text.ElideRight
            textFormat: Text.RichText
            maximumLineCount: authorRect.maximumDescriptionLineCount
            showEllipsis: false
            color: guiSettings.textColor
            plainText: postUtils.linkiFy(author.description, guiSettings.linkColor)
            visible: showAuthor && author.description

            LinkCatcher {
                userDid: authorRect.userDid
            }
        }

        Rectangle {
            Layout.columnSpan: 3
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: authorRect.highlight ? guiSettings.separatorHighLightColor : guiSettings.separatorColor
        }

        Text {
            Layout.columnSpan: 3
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            topPadding: 10
            bottomPadding: 20
            elide: Text.ElideRight
            color: guiSettings.textColor
            text: qsTr("End of list")
            font.italic: true
            visible: endOfList
        }
    }
    MouseArea {
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
                    qsTr(`Do you really want to delete: @${author.handle} ?`),
                    () => deleteItem(listItemUri))
    }

    function authorVisible() {
        return guiSettings.contentVisible(author, userDid)
    }
}
