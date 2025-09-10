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
        root.enablePopupShield(true)
        isMuted = skywalker.mutedWords.containsEntry(hashtag)
        isPinned = skywalker.favoriteFeeds.isPinnedSearch(hashtag)
    }

    onAboutToHide: {
        root.enablePopupShield(false)
        done()
    }

    CloseMenuItem {
        text: qsTr("<b>Hashtags</b>")
        Accessible.name: qsTr("close hashtag options menu")
    }

    AccessibleMenuItem {
        text: qsTr("Focus hashtag")
        onTriggered: focusHashtag(hashtag)
        MenuItemSvg { svg: SvgOutline.hashtag }
    }

    AccessibleMenuItem {
        text: hashtagMenu.isMuted ? qsTr("Unmute hashtag") : qsTr("Mute hashtag")
        onTriggered: {
            if (hashtagMenu.isMuted)
                unmuteWord(hashtag)
            else
                muteWord(hashtag)
        }

        MenuItemSvg { svg: hashtagMenu.isMuted ? SvgOutline.unmute : SvgOutline.mute }
    }

    AccessibleMenuItem {
        text: hashtagMenu.isPinned ? qsTr("Remove favorite") : qsTr("Add favorite")
        onTriggered: {
            const view = searchUtils.createSearchFeed(hashtag,
                postAuthorUser, postMentionsUser,
                postSetSince ? postSince : nullDate,
                postSetUntil ? postUntil : nullDate,
                postLanguage)

            skywalker.favoriteFeeds.pinSearch(view, !hashtagMenu.isPinned)
            skywalker.saveFavoriteFeeds()
        }

        MenuItemSvg {
            svg: hashtagMenu.isPinned ? SvgFilled.star : SvgOutline.star
            color: hashtagMenu.isPinned ? guiSettings.favoriteColor : guiSettings.textColor
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
