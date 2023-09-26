import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Rectangle {
    property int margin: 8
    required property int viewWidth

    required property basicprofile notificationAuthor
    required property list<basicprofile> notificationOtherAuthors
    required property int notificationReason // QEnums::NotificationReason
    required property string notificationReasonSubjectUri
    required property date notificationTimestamp
    required property bool notificationIsRead
    required property string notificationPostText
    required property bool endOfList

    id: notification
    width: grid.width
    height: grid.height
    color: notificationIsRead ? "transparent" : guiSettings.postHighLightColor

    GridLayout {
        id: grid
        columns: 2
        width: viewWidth
        rowSpacing: 5

        // Author and content
        Rectangle {
            id: avatar
            width: guiSettings.threadBarWidth * 5
            height: avatarImg.height + 5
            Layout.fillHeight: true
            color: "transparent"

            Avatar {
                id: avatarImg
                x: parent.x + 8
                y: postHeader.y + 5
                width: parent.width - 13
                height: width
                avatarUrl: notificationAuthor.avatarUrl
                visible: showPost()
            }
            SvgImage {
                x: parent.x + 14
                y: height + 5
                width: parent.width - 19
                height: width
                color: "palevioletred"
                svg: svgFilled.like
                visible: notificationReason === QEnums.NOTIFICATION_REASON_LIKE
            }
            SvgImage {
                x: parent.x + 14
                y: height + 5
                width: parent.width - 19
                height: width
                color: guiSettings.textColor
                svg: svgOutline.repost
                visible: notificationReason === QEnums.NOTIFICATION_REASON_REPOST
            }
            Rectangle {
                x: parent.x + 14
                y: parent.y + 5
                width: parent.width - 19
                height: width
                radius: height / 2
                color: "blue"
                visible: notificationReason === QEnums.NOTIFICATION_REASON_FOLLOW

                SvgImage {
                    x: 5
                    y: height + 5
                    width: parent.width - 10
                    height: width
                    color: "white"
                    svg: svgFilled.newFollower
                }
            }
        }

        Column {
            id: postColumn
            width: parent.width - avatar.width - notification.margin * 2
            visible: showPost()
            topPadding: 5

            PostHeader {
                id: postHeader
                width: parent.width
                Layout.fillWidth: true
                authorName: notificationAuthor.name
                authorHandle: notificationAuthor.handle
                postThreadType: QEnums.THREAD_NONE
                postIndexedSecondsAgo: (new Date() - notificationTimestamp) / 1000
            }

            PostBody {
                id: postBody
                width: parent.width
                Layout.fillWidth: true
                postText: notificationPostText
                postImages: []
                postDateTime: notificationTimestamp
            }
        }
        Column {
            width: parent.width - avatar.width - notification.margin * 2
            topPadding: 5
            visible: isAggregatableReason()

            Row {
                spacing: 5
                Avatar {
                    width: 24
                    height: width
                    avatarUrl: notificationAuthor.avatarUrl
                }
                Repeater {
                    model: Math.min(notificationOtherAuthors.length, 4)

                    Avatar {
                        required property int index

                        width: 24
                        height: width
                        avatarUrl: notificationOtherAuthors[index].avatarUrl
                    }
                }
                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    text: `+${(notificationOtherAuthors.length - 5)}`
                    visible: notificationOtherAuthors.length > 5
                }
            }

            RowLayout {
                width: parent.width

                Text {
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                    text: {
                        `<b>${notificationAuthor.name}</b> ` +
                        (notificationOtherAuthors.length > 0 ?
                            (notificationOtherAuthors.length > 1 ?
                                qsTr(`and ${(notificationOtherAuthors.length)} others `) :
                                qsTr(`and <b>${(notificationOtherAuthors[0].name)}</b> `)) :
                            "") +
                        reasonText()
                    }
                }
                Text {
                    text: guiSettings.durationToString((new Date() - notificationTimestamp) / 1000)
                    font.pointSize: guiSettings.scaledFont(7/8)
                    color: Material.color(Material.Grey)
                }
            }
        }

        // Separator
        Rectangle {
            width: parent.width
            Layout.columnSpan: 2
            Layout.preferredHeight: 1
            Layout.fillWidth: true
            color: "lightgrey"
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
            visible: endOfList
        }
    }

    GuiSettings {
        id: guiSettings
    }

    function showPost() {
        let reasons = [QEnums.NOTIFICATION_REASON_MENTION,
                       QEnums.NOTIFICATION_REASON_REPLY,
                       QEnums.NOTIFICATION_REASON_QUOTE]
        return reasons.includes(notificationReason)
    }

    function isAggregatableReason() {
        let reasons = [QEnums.NOTIFICATION_REASON_LIKE,
                       QEnums.NOTIFICATION_REASON_FOLLOW,
                       QEnums.NOTIFICATION_REASON_REPOST]
        return reasons.includes(notificationReason)
    }

    function reasonText() {
        switch (notificationReason) {
        case QEnums.NOTIFICATION_REASON_LIKE:
            return qsTr("liked your post")
        case QEnums.NOTIFICATION_REASON_FOLLOW:
            return qsTr("started following you")
        case QEnums.NOTIFICATION_REASON_REPOST:
            return qsTr("reposted your post")
        default:
            return "UNKNOW REASON: " + notificationReason
        }
    }
}
