import QtQuick
import QtQuick.Controls
import skywalker

SkyMenu {
    property string userDid
    property Skywalker skywalker: root.getSkywalker(userDid)
    property UserSettings userSettings: skywalker.getUserSettings()
    required property var postFeedModel
    required property var feed
    property bool feedHideFollowing: false
    property bool feedSync: false

    signal showFeed
    signal newReverseFeed(bool reverse)
    signal enableSyncSearchFeed(bool enbable)

    id: feedOptionsMenu
    menuWidth: 270

    SkyMenuButton {
        text: postFeedModel.feedType === QEnums.FEED_GENERATOR ? qsTr("Feed profile") : qsTr("Search")
        svg: postFeedModel.feedType === QEnums.FEED_GENERATOR ? SvgOutline.feed : SvgOutline.search
        popup: feedOptionsMenu
        onClicked: showFeed()
    }

    SkyMenuButton {
        text: qsTr("Remove favorite")
        svg: SvgFilled.star
        svgColor: guiSettings.favoriteColor
        popup: feedOptionsMenu
        onClicked: {
            if (postFeedModel.feedType === QEnums.FEED_GENERATOR)
                skywalker.favoriteFeeds.pinFeed(feedOptionsMenu.feed, false)
            else
                skywalker.favoriteFeeds.pinSearch(feedOptionsMenu.feed, false)

            skywalker.saveFavoriteFeeds()
        }
    }

    SkyMenuButton {
        text: qsTr("Share")
        svg: SvgOutline.share
        popup: feedOptionsMenu
        visible: postFeedModel.feedType === QEnums.FEED_GENERATOR
        onClicked: skywalker.shareFeed(feedOptionsMenu.feed)
    }

    SkyMenuButton {
        text: qsTr("Filtered posts")
        svg: SvgOutline.hideVisibility
        popup: feedOptionsMenu
        onClicked: root.viewContentFilterStats(postFeedModel)
    }

    MenuSeparator {}

    AccessibleText {
        width: parent.width
        leftPadding: 10
        rightPadding: 10
        elide: Text.ElideRight
        font.bold: true
        text: qsTr("Posts order")
    }

    SkyRadioMenuItem {
        text: qsTr("New to old")
        checked: !model.reverseFeed
        enabled: userSettings.globalFeedOrder === QEnums.FEED_ORDER_PER_FEED
        onTriggered: newReverseFeed(false)
    }

    SkyRadioMenuItem {
        text: qsTr("Old to new")
        checked: model.reverseFeed
        enabled: userSettings.globalFeedOrder === QEnums.FEED_ORDER_PER_FEED
        onTriggered: newReverseFeed(true)
    }

    MenuSeparator {}

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
        visible: postFeedModel.chronological
        onToggled: {
            if (postFeedModel.feedType === QEnums.FEED_GENERATOR) {
                const fu = root.getFeedUtils(userDid)
                fu.syncFeed(feedOptionsMenu.feed.uri, checked)
            }
            else
            {
                enableSyncSearchFeed(checked)
            }

            feedOptionsMenu.feedSync = checked
        }
    }

    function show() {
        if (postFeedModel.feedType === QEnums.FEED_GENERATOR) {
            feedHideFollowing = userSettings.getFeedHideFollowing(skywalker.getUserDid(), feed.uri)
            feedSync = userSettings.mustSyncFeed(skywalker.getUserDid(), feed.uri)
        } else {
            feedSync = userSettings.mustSyncSearchFeed(skywalker.getUserDid(), feed.searchQuery)
        }

        open()
    }
}
