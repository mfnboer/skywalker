import QtQuick
import QtQuick.Controls
import skywalker

SkyMenu {
    property string hashtag: "" // or cashtag starting with $
    readonly property bool isCashtag: hashtag.startsWith("$")
    property string handle
    property bool isMuted: false
    property bool isPinned: false
    property var skywalker: root.getSkywalker()

    // Search scope parameters
    property string postAuthorUser // empty, "me", handle
    property string postMentionsUser // empty, "me", handle
    property date postSince
    property bool postSetSince: false
    property date postUntil
    property bool postSetUntil: false
    property string postLanguage
    property date nullDate

    signal done

    id: hashtagMenu
    menuWidth: 250

    onAboutToShow: {
        isMuted = skywalker.mutedWords.containsEntry(hashtag)
        isPinned = skywalker.favoriteFeeds.isPinnedSearch(hashtag)
    }

    onAboutToHide: {
        done()
    }

    CloseMenuItem {
        text: qsTr(`<b>${hashtag}</b>`)
        Accessible.name: qsTr("close menu")
    }

    AccessibleMenuItem {
        text: qsTr("Posts")
        svg: SvgOutline.chat
        onTriggered: root.viewSearchView(hashtag)
        visible: Boolean(handle)
    }

    AccessibleMenuItem {
        text: qsTr("Posts from user")
        svg: SvgOutline.user
        onTriggered: root.viewSearchView(hashtag, handle)
        visible: Boolean(handle)
    }

    AccessibleMenuItem {
        text: hashtagMenu.isCashtag ? qsTr("Focus cashtag") : qsTr("Focus hashtag")
        svg: SvgOutline.hashtag
        onTriggered: focusHashtag(hashtag)
    }

    AccessibleMenuItem {
        text: getText()
        svg: hashtagMenu.isMuted ? SvgOutline.unmute : SvgOutline.mute
        onTriggered: {
            if (hashtagMenu.isMuted)
                unmuteWord(hashtag)
            else
                muteWord(hashtag)
        }

        function getText() {
            if (hashtagMenu.isCashtag)
                return hashtagMenu.isMuted ? qsTr("Unmute cashtag") : qsTr("Mute cashtag")
            else
                return hashtagMenu.isMuted ? qsTr("Unmute hashtag") : qsTr("Mute hashtag")
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
        const entry = isCashtag ? hashtag : hashtag.slice(1) // strip #-symbol
        skywalker.focusHashtags.addEntry(entry)
        root.pushStack(focusPage)
    }

    function show(hashtag, handle = "") {
        hashtagMenu.hashtag = hashtag
        hashtagMenu.handle = handle
        open()
    }
}
