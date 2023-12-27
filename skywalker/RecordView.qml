import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Item {
    property recordview record

    width: parent.width
    height: recordColumn.height + 10

    Column {
        id: recordColumn
        width: parent.width - 10
        anchors.centerIn: parent

        RowLayout {
            width: parent.width
            visible: record.available

            Avatar {
                id: avatar
                width: 24
                Layout.alignment: Qt.AlignTop
                avatarUrl: record.author.avatarUrl

                onClicked: skywalker.getDetailedProfile(record.author.did)
            }

            PostHeader {
                Layout.fillWidth: true
                authorName: record.author.name
                authorHandle: record.author.handle
                postThreadType: QEnums.THREAD_NONE
                postIndexedSecondsAgo: (new Date() - record.postDateTime) / 1000
            }
        }

        PostBody {
            width: parent.width
            postText: record.postTextFormatted
            postImages: record.images
            postContentLabels: record.contentLabels
            postContentVisibility: record.contentVisibility
            postContentWarning: record.contentWarning
            postMuted: record.mutedReason
            postExternal: record.external
            postDateTime: record.postDateTime
            maxTextLines: 6
            visible: record.available
        }

        QuoteFeed {
            width: parent.width
            feed: record.feed
            visible: record.feedAvailable
        }

        Text {
            width: parent.width
            color: guiSettings.textColor
            text: qsTr("NOT FOUND")
            visible: record.notFound
        }
        Text {
            width: parent.width
            color: guiSettings.textColor
            text: qsTr("BLOCKED")
            visible: record.blocked
        }
        Text {
            width: parent.width
            color: guiSettings.textColor
            text: qsTr("NOT SUPPORTED")
            visible: record.notSupported
        }
        Text {
            width: parent.width
            color: guiSettings.textColor
            wrapMode: Text.Wrap
            maximumLineCount: 2
            elide: Text.ElideRight
            font.pointSize: guiSettings.scaledFont(7/8)
            text: record.unsupportedType
            visible: record.notSupported
        }
    }
    Rectangle {
        anchors.fill: parent
        border.width: 1
        border.color: guiSettings.borderColor
        color: "transparent"
        radius: 10
    }
    MouseArea {
        z: -1 // Let other mouse areas, e.g. images, get on top
        anchors.fill: parent
        onClicked: {
            if (record.postUri)
                skywalker.getPostThread(record.postUri)
            else if (record.feedAvailable)
                root.viewPostFeed(record.feed)
        }
    }

    GuiSettings {
        id: guiSettings
    }
}
