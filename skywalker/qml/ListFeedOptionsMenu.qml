import QtQuick
import skywalker

SkyMenu {
    property string userDid
    property Skywalker skywalker: root.getSkywalker(userDid)
    property var userSettings: skywalker.getUserSettings()
    required property var postFeedModel
    required property var list
    property bool listHideFromTimeline: false
    property bool listHideReplies: false
    property bool listHideFollowing: false
    property bool listSync: false

    signal showFeed
    signal newReverseFeed(bool reverse)

    id: listFeedOptionsMenu
    menuWidth: 250

    CloseMenuItem {
        text: qsTr("<b>List</b>")
        Accessible.name: qsTr("close more options menu")
    }

    AccessibleMenuItem {
        text: qsTr("List profile")
        svg: SvgOutline.list
        onTriggered: showFeed()
    }

    AccessibleMenuItem {
        text: qsTr("Remove favorite")
        svg: SvgFilled.star
        svgColor: guiSettings.favoriteColor
        onTriggered: {
            const favorite = skywalker.favoriteFeeds.getPinnedFeed(listFeedOptionsMenu.list.uri)

            if (favorite.isNull()) {
                console.warn("List is not a favorite:" << listFeedOptionsMenu.list.uri)
                return
            }

            if (listFeedOptionsMenu.isOwnList())
                skywalker.favoriteFeeds.removeList(favorite.listView) // We never show own lists as saved
            else
                skywalker.favoriteFeeds.pinList(favorite.listView, false)

            skywalker.saveFavoriteFeeds()
        }
    }

    AccessibleMenuItem {
        id: hideListMenuItem
        visible: listFeedOptionsMenu.list?.purpose === QEnums.LIST_PURPOSE_CURATE && listFeedOptionsMenu.isOwnList()
        text: listFeedOptionsMenu.listHideFromTimeline ? qsTr("Unhide list from timeline") : qsTr("Hide list from timeline")
        svg: listFeedOptionsMenu.listHideFromTimeline ? SvgOutline.unmute : SvgOutline.mute
        onTriggered: {
            const gu = root.getGraphUtils(userDid)

            if (listFeedOptionsMenu.listHideFromTimeline) {
                gu.unhideList(listFeedOptionsMenu.list.uri)
                listFeedOptionsMenu.listHideFromTimeline = false
            }
            else {
                gu.hideList(listFeedOptionsMenu.list.uri)
            }
        }
    }

    AccessibleMenuItem {
        text: qsTr("Share")
        svg: SvgOutline.share
        onTriggered: {
            const favorite = skywalker.favoriteFeeds.getPinnedFeed(listFeedOptionsMenu.list.uri)

            if (favorite.isNull()) {
                console.warn("List is not a favorite:" << listFeedOptionsMenu.list.uri)
                return
            }

            skywalker.shareList(favorite.listView)
        }
    }

    AccessibleMenuItem {
        text: qsTr("Filtered posts")
        svg: SvgOutline.hideVisibility
        onTriggered: root.viewContentFilterStats(postFeedModel)
    }

    PostsOrderMenu {
        reverseFeed: model.reverseFeed
        onNewReverseFeed: (reverse) => listFeedOptionsMenu.newReverseFeed(reverse)
    }

    AccessibleMenuItem {
        text: qsTr("Show replies")
        checkable: true
        checked: !listFeedOptionsMenu.listHideReplies
        onToggled: {
            const gu = root.getGraphUtils(userDid)
            gu.hideReplies(listFeedOptionsMenu.list.uri, !checked)
            listFeedOptionsMenu.listHideReplies = !checked
        }
    }
    AccessibleMenuItem {
        text: qsTr("Show following")
        checkable: true
        checked: !listFeedOptionsMenu.listHideFollowing
        onToggled: {
            const gu = root.getGraphUtils(userDid)
            gu.hideFollowing(listFeedOptionsMenu.list.uri, !checked)
            listFeedOptionsMenu.listHideFollowing = !checked
        }
    }
    AccessibleMenuItem {
        text: qsTr("Rewind on startup")
        checkable: true
        checked: listFeedOptionsMenu.listSync
        onToggled: {
            const gu = root.getGraphUtils(userDid)
            gu.syncList(listFeedOptionsMenu.list.uri, checked)
            listFeedOptionsMenu.listSync = checked
        }
    }

    function isOwnList() {
        if (!list)
            return false

        const pu = root.getPostUtils(userDid)
        const listCreatorDid = pu.extractDidFromUri(list.uri)
        return skywalker.getUserDid() === listCreatorDid
    }

    function show() {
        listHideFromTimeline = skywalker.getTimelineHide().hasList(list.uri)
        listHideReplies = userSettings.getFeedHideReplies(skywalker.getUserDid(), list.uri)
        listHideFollowing = userSettings.getFeedHideFollowing(skywalker.getUserDid(), list.uri)
        listSync = userSettings.mustSyncFeed(skywalker.getUserDid(), list.uri)
        open()
    }
}
