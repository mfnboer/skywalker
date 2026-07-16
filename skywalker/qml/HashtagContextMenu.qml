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
    property SearchOptions searchOptions

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

    SkyMenuButton {
        text: qsTr("Posts")
        svg: SvgOutline.chat
        popup: hashtagMenu
        onClicked: root.viewSearchView(hashtag)
        visible: Boolean(handle)
    }

    SkyMenuButton {
        text: qsTr("Posts from user")
        svg: SvgOutline.user
        popup: hashtagMenu
        onClicked: root.viewSearchView(hashtag, handle)
        visible: Boolean(handle)
    }

    SkyMenuButton {
        text: hashtagMenu.isCashtag ? qsTr("Focus cashtag") : qsTr("Focus hashtag")
        svg: SvgOutline.focusHashtag
        popup: hashtagMenu
        onClicked: focusHashtag(hashtag)
    }

    SkyMenuButton {
        text: getText()
        svg: hashtagMenu.isMuted ? SvgOutline.unmute : SvgOutline.mute
        popup: hashtagMenu
        onClicked: {
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

    SkyMenuButton {
        text: hashtagMenu.isPinned ? qsTr("Remove favorite") : qsTr("Add favorite")
        svg: hashtagMenu.isPinned ? SvgFilled.star : SvgOutline.star
        popup: hashtagMenu
        svgColor: hashtagMenu.isPinned ? guiSettings.favoriteColor : guiSettings.textColor
        onClicked: {
            const view = searchUtils.createSearchFeed(hashtag, searchOptions)
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
