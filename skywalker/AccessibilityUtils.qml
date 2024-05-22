import QtQuick
import QtQuick.Controls
import skywalker

Item {
    property basicprofile nullAuthor

    GifUtils {
        id: gifUtils
    }

    GuiSettings {
        id: guiSettings
    }

    function getTimeSpeech(date) {
        const time = guiSettings.isToday(date) ?
            date.toLocaleTimeString(Qt.locale(), Locale.ShortFormat) :
            date.toLocaleString(Qt.locale(), Locale.ShortFormat)
        return time
    }

    function getPostSpeech(postIndexedDateTime,
                           author,
                           postPlainText,
                           postImages,
                           postExternal,
                           postRecord,
                           postRecordWithMedia,
                           postRepostedByAuthor,
                           postIsReply, postReplyToAuthor) {
        const time = getTimeSpeech(postIndexedDateTime)
        const reposted = !postRepostedByAuthor.isNull() ? qsTr(`\n\nreposted by ${postRepostedByAuthor.name}`) : ""
        const replyTo = postIsReply ? qsTr(`\n\nreply to ${postReplyToAuthor.name}`) : ""
        let speech = `${author.name}\n\n${time} ${replyTo} ${reposted}\n\n${postPlainText}`

        if (postImages.length === 1)
            speech += qsTr("\n\n1 picture attached")
        else if (postImages.length > 1)
            speech += qsTr(`\n\n${postImages.length} pictures attached`)

        if (postExternal) {
            if (gifUtils.isGif(postExternal.uri))
                speech += qsTr("\n\nGIF image attached")
            else
                speech += qsTr("\n\nlink card attached")
        }

        if (postRecord)
            speech += qsTr("\n\nQuoted post attached")

        if (postRecordWithMedia) {
            if (postRecordWithMedia.images.length > 0) {
                speech += qsTr("\n\npictures and quoted post attached")
            }
            else if (postRecordWithMedia.external) {
                if (gifUtils.isGif(postRecordWithMedia.external.uri))
                    speech += qsTr("\n\nGIF image and quoted post attached")
                else
                    speech += qsTr("\n\nlink card and quoted post attached")
            }
        }

        return speech
    }

    function getPostNotAvailableSpeech(postNotFound, postBlocked, postNotSupported) {
        if (postNotFound)
            return qsTr("post not found")

        if (postBlocked)
            return qsTr("post blocked")

        if (postNotSupported)
            return qsTr("not supported")

        return ""
    }

    function getFeedSpeech(feed) {
        let speech = qsTr(`feed: ${feed.displayName} by ${feed.creator.name}\n\n${feed.description}`)
        return speech
    }

    function getListSpeech(list) {
        let speech = qsTr(`${(guiSettings.listTypeName(list.purpose))} by ${list.creator.name}\n\n${list.description}`)
        return speech
    }

    function getContentPrefVisibilitySpeech(pref) {
        switch (pref) {
        case QEnums.CONTENT_PREF_VISIBILITY_HIDE:
            return "hide"
        case QEnums.CONTENT_PREF_VISIBILITY_WARN:
            return "warn"
        case QEnums.CONTENT_PREF_VISIBILITY_SHOW:
            return "show"
        }
    }

    function statSpeech(stat, textSingular, textPlural) {
        if (stat === 0)
            return ""

        if (stat === 1)
            return `, 1 ${textSingular}`

        return `, ${stat} ${textPlural}`
    }
}
