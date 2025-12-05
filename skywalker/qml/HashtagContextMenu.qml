import QtQuick
import QtQuick.Controls
import skywalker

SkyMenu {
    property string hashtag: ""
    property bool isMuted: false
    property bool isPinned: false
    property var skywalker: root.getSkywalker()

    // Search scope parameters
    property string postAuthorUser // empty, "me", handle
    property string postMentionsUser // empty, "me", handle
    property date postSince
    property date postUntil
    property string postLanguage
    property date nullDate

    signal done

    id: hashtagMenu

    onAboutToShow: {
        isMuted = skywalker.mutedWords.containsEntry(hashtag)
        isPinned = skywalker.favoriteFeeds.isPinnedSearch(hashtag)
    }

    onAboutToHide: {
        done()
    }

    CloseMenuItem {
        text: qsTr("<b>Hashtags</b>")
        Accessible.name: qsTr("close hashtag options menu")
    }

    AccessibleMenuItem {
        text: qsTr("Focus hashtag")
        svg: SvgOutline.hashtag
        onTriggered: focusHashtag(hashtag)
    }

    AccessibleMenuItem {
        text: hashtagMenu.isMuted ? qsTr("Unmute hashtag") : qsTr("Mute hashtag")
        svg: hashtagMenu.isMuted ? SvgOutline.unmute : SvgOutline.mute
        onTriggered: {
            if (hashtagMenu.isMuted)
                unmuteWord(hashtag)
            else
                muteWord(hashtag)
        }
    }

    AccessibleMenuItem {
        text: hashtagMenu.isPinned ? qsTr("Remove favorite") : qsTr("Add favorite")
        svg: hashtagMenu.isPinned ? SvgFilled.star : SvgOutline.star
        svgColor: hashtagMenu.isPinned ? guiSettings.favoriteColor : guiSettings.textColor
        onTriggered: {
            const view = searchUtils.createSearchFeed(hashtag,
                postAuthorUser, postMentionsUser,
                postSetSince ? postSince : nullDate,
                postSetUntil ? postUntil : nullDate,
                postLanguage)

            skywalker.favoriteFeeds.pinSearch(view, !hashtagMenu.isPinned)
            skywalker.saveFavoriteFeeds()
        }
    }

    SearchUtils {
        id: searchUtils
        skywalker: hashtagMenu.skywalker
    }

    function muteWord(word) {
        skywalker.mutedWords.addEntry(word)
        skywalker.saveMutedWords()
        skywalker.showStatusMessage(qsTr(`Muted ${word}`), QEnums.STATUS_LEVEL_INFO)
    }

    function unmuteWord(word) {
        skywalker.mutedWords.removeEntry(word)
        skywalker.saveMutedWords()
        skywalker.showStatusMessage(qsTr(`Unmuted ${word}`), QEnums.STATUS_LEVEL_INFO)
    }

    function focusHashtag(hashtag) {
        let component = guiSettings.createComponent("FocusHashtags.qml")
        let focusPage = component.createObject(root)
        let p = root
        focusPage.onClosed.connect(() => { p.popStack() }) // qmllint disable missing-property
        skywalker.focusHashtags.addEntry(hashtag.slice(1)) // strip #-symbol
        root.pushStack(focusPage)
    }

    function show(hashtag) {
        hashtagMenu.hashtag = hashtag
        open()
    }
}
