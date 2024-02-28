import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Item {
    property recordview record

    width: parent.width
    height: recordColumn.height + 10

    Accessible.role: Accessible.Button
    Accessible.name: getSpeech()
    Accessible.onPressAction: showRecord()

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

        // Reply to
        ReplyToRow {
            width: parent.width
            authorName: record.replyToAuthor.name
            visible: record.postIsReply
        }

        PostBody {
            width: parent.width
            postAuthor: record.author
            postText: record.postTextFormatted
            postImages: record.images
            postContentLabels: record.contentLabels
            postContentVisibility: record.contentVisibility
            postContentWarning: record.contentWarning
            postMuted: record.mutedReason
            postExternal: record.external
            postDateTime: record.postDateTime
            visible: record.available
        }

        QuoteFeed {
            width: parent.width
            feed: record.feed
            visible: record.feedAvailable

            Accessible.ignored: true
        }

        QuoteList {
            width: parent.width
            list: record.list
            visible: record.listAvailable

            Accessible.ignored: true
        }

        Text {
            width: parent.width
            color: guiSettings.textColor
            text: qsTr("NOT FOUND")
            visible: record.notFound

            Accessible.ignored: true
        }
        Text {
            width: parent.width
            color: guiSettings.textColor
            text: qsTr("BLOCKED")
            visible: record.blocked

            Accessible.ignored: true
        }
        Text {
            width: parent.width
            color: guiSettings.textColor
            text: qsTr("NOT SUPPORTED")
            visible: record.notSupported

            Accessible.ignored: true
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

            Accessible.ignored: true
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
        onClicked: showRecord()
    }

    AccessibilityUtils {
        id: accessibilityUtils
    }

    function showRecord() {
        if (record.postUri)
            skywalker.getPostThread(record.postUri)
        else if (record.feedAvailable)
            root.viewPostFeed(record.feed)
        else if (record.listAvailable)
            root.viewList(record.list)
    }

    function getSpeech() {
        if (record.available && record.postUri) {
            let speech = accessibilityUtils.getPostSpeech(record.postDateTime, record.author,
                    record.postPlainText, record.images, record.external, null, null,
                    accessibilityUtils.nullAuthor, false, accessibilityUtils.nullAuthor)
            return qsTr(`quoted post\n\n${speech}`)
        }
        else if (record.feedAvailable) {
            return accessibilityUtils.getFeedSpeech(record.feed)
        }
        else if (record.listAvailable) {
            return accessibilityUtils.getListSpeech(record.list)
        }

        return accessibilityUtils.getPostNotAvailableSpeech(
                record.notFound, record.blocked, record.notSupported)
    }

    GuiSettings {
        id: guiSettings
    }
}
