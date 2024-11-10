import QtQuick
import QtQuick.Layouts
import skywalker


Rectangle {
    required property profile author
    required property string followingUri
    required property string blockingUri
    required property string listItemUri // empty when the author list is not an item list
    required property bool authorMuted
    required property bool mutedReposts
    property bool allowDeleteItem: false
    property bool showAuthor: authorVisible()
    property bool showFollow: true
    property bool highlight: false
    property int maximumDescriptionLineCount: 25
    readonly property int margin: 10

    signal follow(basicprofile profile)
    signal unfollow(string did, string uri)
    signal deleteItem(string listItemUri)

    id: authorRect
    height: grid.height
    color: highlight ? guiSettings.postHighLightColor : guiSettings.backgroundColor

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
            Layout.rowSpan: 2
            Layout.preferredWidth: guiSettings.threadColumnWidth
            Layout.fillHeight: true
            color: "transparent"

            Accessible.ignored: true

            Avatar {
                id: avatarImg
                x: parent.x + 8
                y: parent.y + 5
                width: parent.width - 13
                height: width
                author: authorRect.author
                onClicked: skywalker.getDetailedProfile(author.did)
            }
        }

        Column {
            Layout.fillWidth: true
            spacing: 3

            SkyCleanedTextLine {
                width: parent.width
                elide: Text.ElideRight
                font.bold: true
                color: guiSettings.textColor
                plainText: author.name

                Accessible.ignored: true
            }
            Text {
                width: parent.width
                elide: Text.ElideRight
                font.pointSize: guiSettings.scaledFont(7/8)
                color: guiSettings.handleColor
                text: `@${author.handle}`

                Accessible.ignored: true
            }

            ContentLabels {
                id: contentLabels
                anchors.left: parent.left
                anchors.right: undefined
                contentLabels: author.labels
                contentAuthorDid: author.did
            }

            Row {
                spacing: 5

                SkyLabel {
                    text: qsTr("follows you")
                    visible: author.viewer.followedBy && showFollow
                }
                SkyLabel {
                    text: qsTr("blocked")
                    visible: blockingUri && author.viewer.blockingByList.isNull()
                }
                SkyLabel {
                    text: qsTr("list blocked")
                    visible: !author.viewer.blockingByList.isNull()
                }
                SkyLabel {
                    text: qsTr("muted")
                    visible: authorMuted && author.viewer.mutedByList.isNull()
                }
                SkyLabel {
                    text: qsTr("list muted")
                    visible: !author.viewer.mutedByList.isNull()
                }
                SkyLabel {
                    text: qsTr("muted reposts")
                    visible: mutedReposts
                }
            }
        }

        Row {
            Layout.fillHeight: true
            Layout.rightMargin: authorRect.margin

            SkyButton {
                text: qsTr("Follow")
                visible: !followingUri && !isUser(author) && showAuthor && showFollow && !author.associated.isLabeler
                onClicked: follow(author)
                Accessible.name: qsTr(`press to follow ${author.name}`)
            }
            SkyButton {
                flat: true
                text: qsTr("Following")
                visible: followingUri && !isUser(author) && showAuthor && showFollow && !author.associated.isLabeler
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
            rightPadding: 10
            Layout.columnSpan: 2
            Layout.fillWidth: true
            Layout.rightMargin: authorRect.margin
            wrapMode: Text.Wrap
            elide: Text.ElideRight
            maximumLineCount: authorRect.maximumDescriptionLineCount
            color: guiSettings.textColor
            plainText: author.description
            visible: showAuthor && author.description
        }

        Rectangle {
            Layout.columnSpan: 3
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: authorRect.highlight ? guiSettings.separatorHighLightColor : guiSettings.separatorColor
        }
    }
    MouseArea {
        z: -1
        anchors.fill: parent
        onClicked: skywalker.getDetailedProfile(author.did)
    }


    function confirmDelete() {
        guiSettings.askYesNoQuestion(
                    authorRect,
                    qsTr(`Do you really want to delete: @${author.handle} ?`),
                    () => deleteItem(listItemUri))
    }

    function authorVisible() {
        return guiSettings.contentVisible(author)
    }

    function isUser(author) {
        return skywalker.getUserDid() === author.did
    }
}
