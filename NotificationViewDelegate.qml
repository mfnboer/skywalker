import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Rectangle {
    property int margin: 8
    required property int viewWidth

    required property basicprofile notificationAuthor
    required property int notificationReason // QEnums::NotificationReason
    required property string notificationReasonSubjectUri
    required property date notificationTimestamp
    required property bool notificationIsRead
    required property string notificationPostText
    required property bool endOfList

    id: notification
    width: grid.width
    height: grid.height
    color: notificationIsRead ? "azure" : "transparent"

    GridLayout {
        id: grid
        columns: 2
        width: viewWidth
        rowSpacing: margin

        // Author and content
        Rectangle {
            id: avatar
            width: guiSettings.threadBarWidth * 5
            height: avatarImg.height + 5
            Layout.fillHeight: true
            opacity: 0.9

            Avatar {
                id: avatarImg
                x: avatar.x + 8
                y: avatar.y + 5
                width: parent.width - 13
                height: width
                avatarUrl: notificationAuthor.avatarUrl
            }
        }

        Column {
            id: postColumn
            width: parent.width - avatar.width - notification.margin * 2

            PostHeader {
                id: postHeader
                width: parent.width
                authorName: notificationAuthor.name
                authorHandle: notificationAuthor.handle
                postThreadType: QEnums.THREAD_NONE
                postIndexedSecondsAgo: (new Date() - notificationTimestamp) / 1000
            }

            PostBody {
                width: parent.width
                postText: notificationPostText
                postImages: []
                postDateTime: notificationTimestamp
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
}
