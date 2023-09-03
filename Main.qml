import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtQuick.Window
import skywalker

Window {
    id: root
    width: 480
    height: 960
    visible: true
    title: qsTr("Skywalker")

    ListView {
        property bool inTopOvershoot: false
        property bool inBottomOvershoot: false
        property int margin: 5

        id: timelineView
        anchors.fill: parent
        spacing: 0
        model: skywalker.timelineModel
        ScrollIndicator.vertical: ScrollIndicator {}

        delegate: GridLayout {
            required property int index
            required property basicprofile author
            required property string postText
            required property int postIndexedSecondsAgo
            required property string postRepostedByName
            required property list<imageview> postImages
            required property var postExternal // externalview (var allows NULL)
            required property var postRecord // recordview
            required property var postRecordWithMedia // record_with_media_view
            required property int postType // QEnums::PostType
            required property int postGapId;
            required property bool postIsReply
            required property bool postParentInThread
            required property basicprofile postReplyToAuthor
            required property bool endOfFeed;

            id: postEntry
            columns: 2
            width: timelineView.width
            rowSpacing: 0

            // Instead of using row spacing, these empty rectangles are used for white space.
            // This way we can color the background for threads.
            RowLayout {
                id: topLeftSpace
                width: avatar.width
                height: timelineView.margin * (postIsReply && !postParentInThread ? 2 : 1)
                spacing: 0

                Repeater {
                    model: 5

                    Rectangle {
                        required property int index

                        width: avatar.width / 5
                        Layout.preferredHeight: topLeftSpace.height
                        color: {
                            switch (postType) {
                            case QEnums.POST_REPLY:
                            case QEnums.POST_LAST_REPLY:
                                return !postParentInThread && index % 2 === 0 ? "transparent" : "lightcyan"
                            default:
                                return "transparent"
                            }
                        }
                    }
                }
            }
            Rectangle {
                width: parent.width - avatar.width - timelineView.margin * 2
                Layout.preferredHeight: topLeftSpace.height
                Layout.fillWidth: true
            }

            // Repost information
            Rectangle {
                width: avatar.width
                color: avatar.color
                visible: postRepostedByName && !postGapId
            }
            Text {
                width: parent.width - avatar.width - timelineView.margin * 2
                Layout.fillWidth: true
                elide: Text.ElideRight
                text: qsTr(`Reposted by ${postRepostedByName}`)
                color: "darkslategrey"
                font.bold: true
                font.pointSize: 8
                visible: postRepostedByName && !postGapId
            }

            // Author and content
            Rectangle {
                id: avatar
                width: 40
                Layout.fillHeight: true

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
                            default:
                                return "transparent"
                            }
                        }
                    }
                    GradientStop {
                        position: 1.0
                        color: postType === QEnums.POST_STANDALONE ? "transparent" : "lightcyan"
                    }
                }

                Avatar {
                    x: parent.x + 5
                    y: parent.y
                    width: parent.width - 10
                    avatarUrl: author.avatarUrl
                    visible: !postGapId
                }
            }
            Column {
                id: postColumn
                width: parent.width - avatar.width - timelineView.margin * 2
                visible: !postGapId

                PostHeader {
                    width: parent.width
                    authorName: author.name
                    postIndexedSecondsAgo: postEntry.postIndexedSecondsAgo
                }

                // Reply to
                Row {
                    width: parent.width

                    SvgImage {
                        width: replyToText.height
                        height: replyToText.height
                        color: "darkslategrey"
                        svg: svgOutline.reply
                    }

                    Text {
                        id: replyToText
                        width: parent.width
                        elide: Text.ElideRight
                        color: "darkslategrey"
                        font.pointSize: 8
                        text: qsTr(`Reply to ${postReplyToAuthor.name}`)
                    }
                    visible: postIsReply && (!postParentInThread || postType === QEnums.POST_ROOT)
                }

                PostBody {
                    width: parent.width
                    postText: postEntry.postText
                    postImages: postEntry.postImages
                    postExternal: postEntry.postExternal
                    postRecord: postEntry.postRecord
                    postRecordWithMedia: postEntry.postRecordWithMedia
                }

                // Stats
                Row {
                    width: parent.width
                    topPadding: 5

                    StatIcon {
                        width: parent.width / 4
                        svg: svgOutline.reply
                    }
                    StatIcon {
                        width: parent.width / 4
                        svg: svgOutline.repost
                    }
                    StatIcon {
                        width: parent.width / 4
                        svg: svgOutline.like
                    }
                    StatIcon {
                        width: parent.width / 4
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

            // Instead of using row spacing, these empty rectangles are used for white space.
            // This way we can color the background for threads.
            Rectangle {
                width: avatar.width
                height: timelineView.margin
                color: {
                    switch (postType) {
                    case QEnums.POST_ROOT:
                    case QEnums.POST_REPLY:
                        return "lightcyan"
                    default:
                        return "transparent"
                    }
                }
            }
            Rectangle {
                width: parent.width - avatar.width - timelineView.margin * 2
                height: timelineView.margin
                Layout.fillWidth: true
            }

            // Post/Thread separator
            Rectangle {
                width: parent.width
                Layout.columnSpan: 2
                Layout.preferredHeight: 1
                Layout.fillWidth: true
                color: "lightgrey"
                visible: [QEnums.POST_STANDALONE, QEnums.POST_LAST_REPLY].includes(postType)
            }

            // End of feed indication
            Text {
                Layout.columnSpan: 2
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                elide: Text.ElideRight
                text: qsTr("End of feed")
                font.italic: true
                visible: endOfFeed
            }
        }

        onCountChanged: {
            let firstVisibleIndex = indexAt(0, contentY)
            let lastVisibleIndex = indexAt(0, contentY + height - 1)
            console.debug("COUNT CHANGED First:", firstVisibleIndex, "Last:", lastVisibleIndex, "Count:", count)
            // Adding/removing content changes the indices.
            skywalker.timelineMovementEnded(firstVisibleIndex, lastVisibleIndex)
        }

        onMovementEnded: {
            let firstVisibleIndex = indexAt(0, contentY)
            let lastVisibleIndex = indexAt(0, contentY + height - 1)
            console.debug("END MOVEMENT First:", firstVisibleIndex, "Last:", lastVisibleIndex, "Count:", count)
            skywalker.timelineMovementEnded(firstVisibleIndex, lastVisibleIndex)
        }

        onVerticalOvershootChanged: {
            if (verticalOvershoot < 0)  {
                if (!inTopOvershoot && !skywalker.getTimelineInProgress) {
                    skywalker.getTimeline(50)
                }

                inTopOvershoot = true
            } else {
                inTopOvershoot = false
            }

            if (verticalOvershoot > 0) {
                if (!inBottomOvershoot && !skywalker.getTimelineInProgress) {
                    skywalker.getTimelineNextPage()
                }

                inBottomOvershoot = true;
            } else {
                inBottomOvershoot = false;
            }
        }
    }

    SvgOutline {
        id: svgOutline
    }

    Login {
        id: loginDialog
        anchors.centerIn: parent
        onAccepted: skywalker.login(user, password, host)

    }

    Skywalker {
        id: skywalker
        onLoginOk: {
            skywalker.getTimeline(50)
            timelineUpdateTimer.start()
        }
        onLoginFailed: (error) => loginDialog.show(error)
    }

    Timer {
        id: timelineUpdateTimer
        interval: 30000
        running: false
        repeat: true
        onTriggered: skywalker.getTimelinePrepend(2)
    }

    function viewFullImage(imageList, currentIndex) {
        let component = Qt.createComponent("FullImageView.qml")
        let view = component.createObject(root, {images: imageList, imageIndex: currentIndex})
        view.open()
    }

    Component.onCompleted: {
        loginDialog.show()
    }
}
