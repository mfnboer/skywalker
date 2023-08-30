import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import QtQuick.Window
import skywalker

Window {
    width: 480
    height: 960
    visible: true
    title: qsTr("Skywalker")

    ListView {
        property bool inTopOvershoot: false
        property bool inBottomOvershoot: false

        id: timelineView
        anchors.fill: parent
        spacing: 5
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
            required property int postGapId;
            required property bool endOfFeed;

            id: postEntry
            columns: 2
            width: timelineView.width

            Rectangle {
                width: avatar.width
                color: "transparent"
                visible: postRepostedByName && !postGapId
            }

            Text {
                width: parent.width - avatar.width - timelineView.spacing * 2
                Layout.fillWidth: true
                elide: Text.ElideRight
                text: qsTr(`Reposted by ${postRepostedByName}`)
                color: "darkslategrey"
                font.bold: true
                font.pointSize: 8
                visible: postRepostedByName && !postGapId
            }

            Avatar {
                id: avatar
                width: 30
                Layout.alignment: Qt.AlignTop
                avatarUrl: author.avatarUrl
                visible: !postGapId
            }

            Column {
                id: postColumn
                width: parent.width - avatar.width - timelineView.spacing * 2
                visible: !postGapId

                PostHeader {
                    width: parent.width
                    authorName: author.name
                    postIndexedSecondsAgo: postEntry.postIndexedSecondsAgo
                }

                PostBody {
                    width: parent.width
                    postText: postEntry.postText
                    postImages: postEntry.postImages
                    postExternal: postEntry.postExternal
                    postRecord: postEntry.postRecord
                    postRecordWithMedia: postEntry.postRecordWithMedia
                }
            }

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
            }

            Rectangle {
                width: parent.width
                Layout.columnSpan: 2
                Layout.preferredHeight: 1
                Layout.fillWidth: true
                color: "lightgrey"
            }

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

        onMovementEnded: {
            let lastVisibleIndex = indexAt(0, contentY + height - 1)
            console.debug("END MOVEMENT", visibleArea.yPosition + visibleArea.heightRatio, lastVisibleIndex, count);
            if (lastVisibleIndex > timelineView.count - 5 && !skywalker.getTimelineInProgress) {
                skywalker.getTimelineNextPage()
            }
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

    Component.onCompleted: {
        loginDialog.show()
    }
}
