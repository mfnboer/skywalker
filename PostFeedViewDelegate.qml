import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Rectangle {
    property int margin: 8
    property int viewWidth
    property date now: new Date()

    required property int index
    required property basicprofile author
    required property string postUri
    required property string postText
    required property date postIndexedDateTime
    required property string postRepostedByName
    required property list<imageview> postImages
    required property var postExternal // externalview (var allows NULL)
    required property var postRecord // recordview
    required property var postRecordWithMedia // record_with_media_view
    required property int postType // QEnums::PostType
    required property int postThreadType // QEnums::ThreadPostType flags
    required property bool postIsPlaceHolder
    required property int postGapId;
    required property bool postNotFound;
    required property bool postBlocked;
    required property bool postNotSupported;
    required property string postUnsupportedType;
    required property bool postIsReply
    required property bool postParentInThread
    required property basicprofile postReplyToAuthor
    required property int postReplyCount
    required property int postRepostCount
    required property int postLikeCount
    required property bool postReposted
    required property bool postLiked
    required property bool endOfFeed;

    id: postEntry
    width: grid.width
    height: grid.height
    color: {
        if (postThreadType & QEnums.THREAD_ENTRY)
            return "azure"
        else
            return "transparent"
    }
    border.width: postThreadType & QEnums.THREAD_ENTRY ? 2 : 0

    GridLayout {
        id: grid
        columns: 2
        width: viewWidth
        rowSpacing: 0

        // Instead of using row spacing, these empty rectangles are used for white space.
        // This way we can color the background for threads.
        RowLayout {
            id: topLeftSpace
            width: avatar.width
            height: postEntry.margin * (postIsReply && !postParentInThread ? 2 : 1)
            spacing: 0

            Repeater {
                model: 11

                Rectangle {
                    required property int index

                    width: avatar.width / 11
                    Layout.preferredHeight: topLeftSpace.height
                    color: {
                        switch (postType) {
                        case QEnums.POST_REPLY:
                        case QEnums.POST_LAST_REPLY:
                            return !postParentInThread && index % 2 === 0 ? "transparent" : "lightcyan"
                        case QEnums.POST_THREAD: {
                            if (postThreadType & QEnums.THREAD_FIRST_DIRECT_CHILD) {
                                return "cyan"
                            } else if ((postThreadType & QEnums.THREAD_DIRECT_CHILD) ||
                                       (postThreadType & QEnums.THREAD_ENTRY)){
                                return "darkcyan"
                            } else if (postThreadType & QEnums.THREAD_TOP) {
                                return "transparent"
                            } else if (postThreadType & QEnums.THREAD_PARENT) {
                                return "cyan"
                            }

                            return "lightcyan"
                        }
                        default:
                            return "transparent"
                        }
                    }
                    opacity: avatar.opacity
                }
            }
        }
        Rectangle {
            width: parent.width - avatar.width - postEntry.margin * 2
            Layout.preferredHeight: topLeftSpace.height
            Layout.fillWidth: true
            color: "transparent"
        }

        // Repost information
        Rectangle {
            width: avatar.width
            height: repostedByText.height
            color: "transparent"
            visible: postRepostedByName && !postGapId

            SvgImage {
                anchors.right: parent.right
                width: repostedByText.height
                height: repostedByText.height
                color: Material.color(Material.Grey)
                svg: svgOutline.repost
            }
        }
        Text {
            id: repostedByText
            width: parent.width - avatar.width - postEntry.margin * 2
            Layout.fillWidth: true
            elide: Text.ElideRight
            text: qsTr(`Reposted by ${postRepostedByName}`)
            color: Material.color(Material.Grey)
            font.bold: true
            font.pointSize: root.scaledFont(7/8)
            visible: postRepostedByName && !postGapId
        }

        // Author and content
        Rectangle {
            id: avatar
            width: 55
            Layout.fillHeight: true
            opacity: 0.9

            // Gradient is used display thread context.
            gradient: Gradient {
                GradientStop {
                    position: 0.0
                    color: {
                        switch (postType) {
                        case QEnums.POST_ROOT:
                            return "cyan"
                        case QEnums.POST_REPLY:
                        case QEnums.POST_LAST_REPLY:
                            return "lightcyan"
                        case QEnums.POST_THREAD: {
                            if (postThreadType & QEnums.THREAD_ENTRY) {
                                return "darkcyan"
                            } else if ((postThreadType & QEnums.THREAD_PARENT) ||
                                    (postThreadType & QEnums.THREAD_DIRECT_CHILD)) {
                                return "cyan"
                            }

                            return "lightcyan"
                        }
                        default:
                            return "transparent"
                        }
                    }
                }
                GradientStop {
                    position: 1.0
                    color: {
                        switch (postType) {
                        case QEnums.POST_STANDALONE:
                            return "transparent"
                        case QEnums.POST_THREAD: {
                            if (postThreadType & QEnums.THREAD_ENTRY) {
                                return "darkcyan"
                            } else if (postThreadType & QEnums.THREAD_PARENT) {
                                return "cyan"
                            }

                            return "lightcyan"
                        }
                        default:
                            return "lightcyan"
                        }
                    }
                }
            }

            Avatar {
                id: avatarImg
                x: avatar.x + 8
                y: postHeader.y + 5 // For some reaon "avatar.y + 5" does not work when it is a repost
                width: parent.width - 13
                avatarUrl: author.avatarUrl
                visible: !postIsPlaceHolder
            }
        }
        Column {
            id: postColumn
            width: parent.width - avatar.width - postEntry.margin * 2
            visible: !postIsPlaceHolder

            PostHeader {
                id: postHeader
                width: parent.width
                authorName: author.name
                authorHandle: author.handle
                postThreadType: postEntry.postThreadType
                postIndexedSecondsAgo: (postEntry.now - postEntry.postIndexedDateTime) / 1000
            }

            // Reply to
            Row {
                width: parent.width

                SvgImage {
                    width: replyToText.height
                    height: replyToText.height
                    color: Material.color(Material.Grey)
                    svg: svgOutline.reply
                }

                Text {
                    id: replyToText
                    width: parent.width
                    elide: Text.ElideRight
                    color: Material.color(Material.Grey)
                    font.pointSize: root.scaledFont(7/8)
                    text: qsTr(`Reply to ${postReplyToAuthor.name}`)
                }

                visible: postIsReply && (!postParentInThread || postType === QEnums.POST_ROOT)
            }

            PostBody {
                width: parent.width
                postUri: postEntry.postUri
                postText: postEntry.postText
                postImages: postEntry.postImages
                postExternal: postEntry.postExternal
                postRecord: postEntry.postRecord
                postRecordWithMedia: postEntry.postRecordWithMedia
                postDateTime: postEntry.postIndexedDateTime
                detailedView: postThreadType & QEnums.THREAD_ENTRY
            }

            // Reposts and likes in detailed view of post entry in thread view
            Row {
               width: parent.width
               topPadding: 10
               bottomPadding: 5
               visible: postThreadType & QEnums.THREAD_ENTRY

               Text {
                   rightPadding: 30
                   textFormat: Text.StyledText
                   text: postRepostCount > 1 ? qsTr(`<b>${postRepostCount}</b> reposts`) : qsTr(`<b>${postRepostCount}</b> repost`)
                   visible: postRepostCount
               }
               Text {
                   textFormat: Text.StyledText
                   text: postLikeCount > 1 ? qsTr(`<b>${postLikeCount}</b> likes`) : qsTr(`<b>${postLikeCount}</b> like`)
                   visible: postLikeCount
               }
            }

            // Stats
            Row {
                width: parent.width
                topPadding: 5

                StatIcon {
                    width: parent.width / 4
                    iconColor: "grey"
                    svg: svgOutline.reply
                    statistic: postReplyCount
                }
                StatIcon {
                    width: parent.width / 4
                    iconColor: postReposted ? "palevioletred" : "grey"
                    svg: svgOutline.repost
                    statistic: postRepostCount
                }
                StatIcon {
                    width: parent.width / 4
                    iconColor: postLiked ? "palevioletred" : "grey"
                    svg: postLiked ? svgFilled.like : svgOutline.like
                    statistic: postLikeCount
                }
                StatIcon {
                    width: parent.width / 4
                    iconColor: "grey"
                    svg: svgOutline.moreVert
                }
            }
        }

        // Gap place holder
        Text {
            width: parent.width
            Layout.columnSpan: 2
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
            textFormat: Text.StyledText
            text: "<a href=\"showMore\">" + qsTr("Show more posts") + "</a>"
            visible: postGapId > 0

            onLinkActivated: {
                if (!skywalker.getTimelineInProgress)
                    skywalker.getTimelineForGap(postGapId)
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
                acceptedButtons: Qt.NoButton
            }
        }

        // NOT FOUND place holder
        Text {
            width: parent.width
            Layout.columnSpan: 2
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
            text: qsTr("NOT FOUND")
            visible: postNotFound
        }

        // BLOCKED place holder
        Text {
            width: parent.width
            Layout.columnSpan: 2
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
            text: qsTr("BLOCKED")
            visible: postBlocked
        }

        // NOT SUPPORTED place holder
        Text {
            width: parent.width
            Layout.columnSpan: 2
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
            text: qsTr("NOT SUPPORTED")
            visible: postNotSupported
        }
        Text {
            width: parent.width
            Layout.columnSpan: 2
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            maximumLineCount: 2
            elide: Text.ElideRight
            color: Material.color(Material.Grey)
            font.pointSize: root.scaledFont(7/8)
            text: postUnsupportedType
            visible: postNotSupported
        }

        // Instead of using row spacing, these empty rectangles are used for white space.
        // This way we can color the background for threads.
        Rectangle {
            width: avatar.width
            height: postEntry.margin
            color: {
                switch (postType) {
                case QEnums.POST_ROOT:
                case QEnums.POST_REPLY:
                    return "lightcyan"
                case QEnums.POST_THREAD: {
                    if (postThreadType & QEnums.THREAD_ENTRY)  {
                        return "darkcyan"
                    }
                    if (postThreadType & QEnums.THREAD_LEAF) {
                        return "transparent"
                    } else if (postThreadType & QEnums.THREAD_PARENT)  {
                        return "cyan"
                    }

                    return "lightcyan"
                }
                default:
                    return "transparent"
                }
            }
            opacity: avatar.opacity
        }
        Rectangle {
            width: parent.width - avatar.width - postEntry.margin * 2
            height: postEntry.margin
            Layout.fillWidth: true
            color: "transparent"
        }

        // Post/Thread separator
        Rectangle {
            width: parent.width
            Layout.columnSpan: 2
            Layout.preferredHeight: 1
            Layout.fillWidth: true
            color: "lightgrey"
            visible: [QEnums.POST_STANDALONE, QEnums.POST_LAST_REPLY].includes(postType) ||
            (postThreadType & QEnums.THREAD_LEAF)
        }

        // End of feed indication
        Text {
            Layout.columnSpan: 2
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
            color: Material.foreground
            text: qsTr("End of feed")
            font.italic: true
            visible: endOfFeed
        }
    }

    MouseArea {
        z: -2 // Let other mouse areas, e.g. images, get on top, -2 to allow records on top
        anchors.fill: parent
        enabled: !((postThreadType & QEnums.THREAD_ENTRY))
        onClicked: {
            console.debug("POST CLICKED:", postUri)
            if (postUri)
                skywalker.getPostThread(postUri)
        }
    }
}
