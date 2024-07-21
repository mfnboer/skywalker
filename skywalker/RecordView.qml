import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Item {
    property recordview record
    property string backgroundColor: "transparent"

    signal opening

    id: recordView
    width: parent.width
    height: recordColumn.height + 10

    Accessible.role: Accessible.Button
    Accessible.name: getSpeech()
    Accessible.onPressAction: showRecord()

    Rectangle {
        anchors.fill: parent
        border.width: 1
        border.color: guiSettings.borderColor
        color: recordView.backgroundColor
        radius: 10
    }

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
                isModerator: record.author.associated.isLabeler

                onClicked: skywalker.getDetailedProfile(record.author.did)
            }

            PostHeader {
                Layout.fillWidth: true
                author: record.author
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

        Loader {
            width: parent.width
            active: record.available

            sourceComponent: PostBody {
                width: parent.width
                postAuthor: record.author
                postText: record.postTextFormatted
                postImages: record.images
                postLanguageLabels: record.languages
                postContentLabels: record.contentLabels
                postContentVisibility: record.contentVisibility
                postContentWarning: record.contentWarning
                postMuted: record.mutedReason
                postExternal: record.external
                postDateTime: record.postDateTime
            }
        }

        Loader {
            width: parent.width
            active: record.feedAvailable

            sourceComponent: QuoteFeed {
                width: parent.width
                feed: record.feed
                Accessible.ignored: true
            }
        }

        Loader {
            width: parent.width
            active: record.listAvailable

            sourceComponent: QuoteList {
                width: parent.width
                list: record.list
                Accessible.ignored: true
            }
        }

        Loader {
            width: parent.width
            active: record.labelerAvailable

            sourceComponent: QuoteLabeler {
                width: parent.width
                labeler: record.labeler
            }
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
        else if (record.labelerAvailable)
            skywalker.getDetailedProfile(record.labeler.creator.did)

        opening()
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
        else if (record.labelerAvailable) {
            return accessibilityUtils.getLabelerSpeech(record.labeler)
        }

        return accessibilityUtils.getPostNotAvailableSpeech(
                record.notFound, record.blocked, record.notSupported)
    }

    GuiSettings {
        id: guiSettings
    }
}
