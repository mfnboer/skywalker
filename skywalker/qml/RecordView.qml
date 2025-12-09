import QtQuick
import QtQuick.Layouts
import skywalker

Item {
    property string userDid
    property Skywalker skywalker: root.getSkywalker(userDid)
    property recordview record
    readonly property bool postBlockedByUser: record.blocked && record.blockedAuthor.viewer.valid && !record.blockedAuthor.viewer.blockedBy &&
            (record.blockedAuthor.viewer.blocking || !record.blockedAuthor.viewer.blockingByList.isNull())
    property bool highlight: false
    property string backgroundColor: "transparent"
    property string borderColor: highlight ? guiSettings.borderHighLightColor : guiSettings.borderColor

    signal opening

    id: recordView
    width: parent.width
    height: recordColumn.height + 20

    Accessible.role: Accessible.Button
    Accessible.name: getSpeech()
    Accessible.onPressAction: showRecord()

    Rectangle {
        anchors.fill: parent
        border.width: 1
        border.color: recordView.borderColor
        color: recordView.backgroundColor
        radius: guiSettings.radius
    }

    Column {
        id: recordColumn
        width: parent.width - 20
        anchors.centerIn: parent

        PostHeaderWithAvatar {
            width: parent.width
            userDid: recordView.userDid
            author: recordView.getRecordAuthor()
            postIndexedSecondsAgo: (new Date() - record.postDateTime) / 1000
            visible: record.available || recordView.postBlockedByUser
        }

        // Reply to
        ReplyToRow {
            width: parent.width
            text: qsTr(`Reply to ${record.replyToAuthor.name}`)
            visible: record.postIsReply
        }

        Loader {
            id: postBodyLoader
            width: parent.width
            active: record.available

            sourceComponent: PostBody {
                width: parent.width
                userDid: recordView.userDid
                postAuthor: recordView.getRecordAuthor()
                postText: record.postTextFormatted
                postPlainText: record.postPlainText
                postHasUnknownEmbed: record.hasUnknownEmbed
                postUnknownEmbedType: record.unknownEmbedType
                postImages: record.images
                postLanguageLabels: record.languages
                postContentLabels: record.contentLabels
                postContentVisibility: record.contentVisibility
                postContentWarning: record.contentWarning
                postContentLabeler: record.contentLabeler
                postMuted: record.mutedReason
                postIsThread: record.postIsThread === QEnums.TRIPLE_BOOL_YES
                postIsThreadReply: record.postIsThreadReply
                postVideo: record.video
                postExternal: record.external
                postDateTime: record.postDateTime
                bodyBackgroundColor: recordView.backgroundColor == "transparent" ? guiSettings.backgroundColor : recordView.backgroundColor

                onUnrollThread: {
                    if (record.postUri)
                        skywalker.getPostThread(record.postUri, QEnums.POST_THREAD_UNROLLED)
                }
            }
        }

        Loader {
            width: parent.width
            active: record.feedAvailable

            sourceComponent: QuoteFeed {
                width: parent.width
                userDid: recordView.userDid
                feed: record.feed
                Accessible.ignored: true
            }
        }

        Loader {
            width: parent.width
            active: record.listAvailable

            sourceComponent: QuoteList {
                width: parent.width
                userDid: recordView.userDid
                list: record.list
                Accessible.ignored: true
            }
        }

        Loader {
            width: parent.width
            active: record.labelerAvailable

            sourceComponent: QuoteLabeler {
                width: parent.width
                userDid: recordView.userDid
                labeler: record.labeler
            }
        }

        Loader {
            width: parent.width
            active: record.starterPackAvailable

            sourceComponent: QuoteStarterPack {
                width: parent.width
                userDid: recordView.userDid
                starterPack: record.starterPack
            }
        }

        Loader {
            width: parent.width
            active: record.notFound || record.notSupported

            sourceComponent: AccessibleText {
                width: parent.width
                text: getDescription()

                Accessible.ignored: true

                function getDescription() {
                    if (record.notFound)
                        return qsTr("🗑 Not found")
                    if (record.notSupported)
                        return qsTr("⚠️ Not supported")
                    return "UNNKNOW"
                }
            }
        }

        Loader {
            width: parent.width
            active: record.blocked && !recordView.postBlockedByUser

            sourceComponent: AccessibleText {
                width: parent.width
                text: qsTr("🚫 Blocked")
            }
        }

        Loader {
            width: parent.width
            active: recordView.postBlockedByUser

            sourceComponent: Column {
                width: parent.width

                AccessibleText {
                    topPadding: 5
                    bottomPadding: 5
                    width: parent.width
                    elide: Text.ElideRight
                    text: {
                        if (record.blockedAuthor.blockingByListUri)
                            return qsTr("🚫 Blocked by list")
                        else if (record.blockedAuthor.viewer.blocking)
                            return qsTr("🚫 Blocked by you")
                        else
                            return qsTr("🚫 Blocked")
                    }
                }

                ListHeader {
                    width: parent.width
                    list: record.blockedAuthor.blockingByList
                    visible: !record.blockedAuthor.blockingByList.isNull()
                }
            }
        }

        Loader {
            width: parent.width
            active: record.detached

            sourceComponent: AccessibleText {
                width: parent.width
                textFormat: Text.RichText
                text: record.detachedByDid === skywalker.getUserDid() ?
                          qsTr("🗑 Detached by you") + ` <a href=\"show\" style=\"color: ${guiSettings.linkColor};\">` + qsTr("Show post") + "</a>" :
                          qsTr("🗑 Detached by author")

                Accessible.ignored: true

                onLinkActivated: skywalker.getPostThread(record.detachedPostUri)
            }
        }

        Loader {
            width: parent.width
            active: record.notSupported

            sourceComponent: AccessibleText {
                width: parent.width
                wrapMode: Text.Wrap
                maximumLineCount: 2
                elide: Text.ElideRight
                font.pointSize: guiSettings.scaledFont(7/8)
                text: record.unsupportedType

                Accessible.ignored: true
            }
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
            root.viewPostFeed(record.feed, recordView.userDid)
        else if (record.listAvailable)
            root.viewList(record.list, recordView.userDid)
        else if (record.labelerAvailable)
            skywalker.getDetailedProfile(record.labeler.creator.did)
        else if (record.starterPackAvailable)
            skywalker.getStarterPackView(record.starterPack.uri)

        opening()
    }

    function getRecordAuthor() {
        return record.blocked ? record.blockedAuthor.author : record.author
    }

    function getSpeech() {
        if (record.available && record.postUri) {
            let speech = accessibilityUtils.getPostSpeech(record.postDateTime, getRecordAuthor(),
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
        else if (record.starterPackAvailable) {
            return accessibilityUtils.getStarterPackSpeech(record.starterPack)
        }

        if (record.detached)
            return record.detachedByDid === skywalker.getUserDid() ? qsTr("quote removed by you") : qsTr("quote removed by author")

        return accessibilityUtils.getPostNotAvailableSpeech(
                record.notFound, record.blocked, record.notSupported)
    }

    function movedOffScreen() {
        if (postBodyLoader.item)
            postBodyLoader.item.movedOffScreen()
    }
}
