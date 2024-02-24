import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Rectangle {
    property int margin: 8
    required property int viewWidth

    required property basicprofile author
    required property string postText
    required property string postPlainText
    required property date postIndexedDateTime
    required property list<imageview> postImages
    required property var postExternal // externalview (var allows NULL)
    required property var postRecord // recordview
    required property var postRecordWithMedia // record_with_media_view
    required property bool postIsReply
    required property basicprofile postReplyToAuthor
    required property list<contentlabel> postLabels
    required property int postContentVisibility // QEnums::PostContentVisibility
    required property string postContentWarning
    required property int postMutedReason // QEnums::MutedPostReason
    required property bool endOfFeed

    signal selected

    id: draftPostView
    width: grid.width
    height: grid.height
    color: "transparent"

    Accessible.role: Accessible.Button
    Accessible.name: getSpeech()
    Accessible.onPressAction: selectDraft()

    GridLayout {
        id: grid
        columns: 2
        width: viewWidth
        rowSpacing: 5

        // Author and content
        Rectangle {
            id: avatar
            width: guiSettings.threadBarWidth * 5
            height: avatarImg.height + 10
            Layout.fillHeight: true
            color: "transparent"

            Avatar {
                id: avatarImg
                x: parent.x + 8
                y: postHeader.y + 5
                width: parent.width - 13
                height: width
                avatarUrl: author.avatarUrl

                onClicked: selectDraft()
            }
        }

        Column {
            id: postColumn
            width: parent.width - avatar.width - draftPostView.margin * 2
            topPadding: 5

            PostHeader {
                id: postHeader
                width: parent.width
                Layout.fillWidth: true
                authorName: author.name
                authorHandle: author.handle
                postThreadType: QEnums.THREAD_NONE
                postIndexedSecondsAgo: (new Date() - postIndexedDateTime) / 1000
            }

            // Reply to
            ReplyToRow {
                width: parent.width
                authorName: postReplyToAuthor.name
                visible: postIsReply
            }

            PostBody {
                id: postBody
                width: parent.width
                Layout.fillWidth: true
                postAuthor: draftPostView.author
                postText: draftPostView.postText
                postPlainText: draftPostView.postPlainText
                postImages: draftPostView.postImages
                postContentLabels: draftPostView.postLabels
                postContentVisibility: draftPostView.postContentVisibility
                postContentWarning: draftPostView.postContentWarning
                postMuted: draftPostView.postMutedReason
                postExternal: draftPostView.postExternal
                postRecord: draftPostView.postRecord
                postRecordWithMedia: draftPostView.postRecordWithMedia
                postDateTime: draftPostView.postIndexedDateTime
            }
        }

        // Separator
        Rectangle {
            width: parent.width
            Layout.columnSpan: 2
            Layout.preferredHeight: 1
            Layout.fillWidth: true
            color: guiSettings.separatorColor
        }

        // End of feed indication
        Text {
            width: parent.width
            Layout.columnSpan: 2
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            topPadding: 10
            elide: Text.ElideRight
            color: guiSettings.textColor
            text: qsTr("End of drafts")
            font.italic: true
            visible: endOfFeed
        }
    }

    MouseArea {
        z: -2 // Let other mouse areas, e.g. images, get on top, -2 to allow records on top
        anchors.fill: parent
        onClicked: selectDraft()
    }

    AccessibilityUtils {
        id: accessibilityUtils
    }

    GuiSettings {
        id: guiSettings
    }

    function getSpeech() {
        return accessibilityUtils.getPostSpeech(postIndexedDateTime, author, postPlainText,
                postImages, postExternal, postRecord, postRecordWithMedia,
                accessibilityUtils.nullAuthor, postIsReply, postReplyToAuthor)
    }

    function selectDraft() {
        selected()
    }
}
