import QtQuick
import skywalker

SkyMenu {
    property string userDid
    property Skywalker skywalker: root.getSkywalker(userDid)
    property UserSettings userSetting: skywalker.getUserSettings()
    required property var postFeedModel
    required property var feed
    property bool feedHideFollowing: false
    property bool feedSync: false

    signal showFeed
    signal newReverseFeed(bool reverse)

    id: feedOptionsMenu
    menuWidth: 270

    CloseMenuItem {
        text: qsTr(`<b>${feed.name}</b>`)
        Accessible.name: qsTr("close more options menu")
    }

    AccessibleMenuItem {
        text: postFeedModel.feedType === QEnums.FEED_GENERATOR ? qsTr("Feed profile") : qsTr("Search")
        svg: postFeedModel.feedType === QEnums.FEED_GENERATOR ? SvgOutline.feed : SvgOutline.search
        onTriggered: showFeed()
    }

    AccessibleMenuItem {
        text: qsTr("Remove favorite")
        svg: SvgFilled.star
        svgColor: guiSettings.favoriteColor
        onTriggered: {
            skywalker.favoriteFeeds.pinFeed(feedOptionsMenu.feed, false)
            skywalker.saveFavoriteFeeds()
        }
    }

    AccessibleMenuItem {
        text: qsTr("Share")
        svg: SvgOutline.share
        visible: postFeedModel.feedType === QEnums.FEED_GENERATOR
        onTriggered: skywalker.shareFeed(feedOptionsMenu.feed)
    }

    AccessibleMenuItem {
        text: qsTr("Filtered posts")
        svg: SvgOutline.hideVisibility
        onTriggered: root.viewContentFilterStats(postFeedModel)
    }

    PostsOrderMenu {
        reverseFeed: model.reverseFeed
        globalFeedOrder: userSettings.globalFeedOrder
        onNewReverseFeed: (reverse) => feedOptionsMenu.newReverseFeed(reverse)
    }

    AccessibleMenuItem {
        text: qsTr("Show following")
        checkable: true
        checked: !feedOptionsMenu.feedHideFollowing
        visible: postFeedModel.feedType === QEnums.FEED_GENERATOR
        onToggled: {
            const fu = root.getFeedUtils(userDid)
            fu.hideFollowing(feedOptionsMenu.feed.uri, !checked)
            feedOptionsMenu.feedHideFollowing = !checked
        }
    }
    AccessibleMenuItem {
        text: qsTr("Rewind on startup")
        checkable: true
        checked: feedOptionsMenu.feedSync
        visible: postFeedModel.feedType === QEnums.FEED_GENERATOR && postFeedModel.chronological
        onToggled: {
            const fu = root.getFeedUtils(userDid)
            fu.syncFeed(feedOptionsMenu.feed.uri, checked)
            feedOptionsMenu.feedSync = checked
        }
    }

    function show() {
        feedHideFollowing = skywalker.getUserSettings().getFeedHideFollowing(skywalker.getUserDid(), feed.uri)
        feedSync = userSettings.mustSyncFeed(skywalker.getUserDid(), feed.uri)
        open()
    }
}
