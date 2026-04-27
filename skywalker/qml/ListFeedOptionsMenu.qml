import QtQuick
import QtQuick.Controls
import skywalker

SkyMenu {
    property string userDid
    property Skywalker skywalker: root.getSkywalker(userDid)
    property var userSettings: skywalker.getUserSettings()
    required property var postFeedModel
    required property listviewbasic list
    property bool isPinnedList: skywalker.favoriteFeeds.isPinnedFeed(list.uri)
    property bool listHideFromTimeline: false
    property bool listHideReplies: false
    property bool listHideFollowing: false
    property bool listSync: false

    signal showFeed
    signal newReverseFeed(bool reverse)

    id: listFeedOptionsMenu
    menuWidth: 270

    SkyMenuButton {
        text: qsTr("List profile")
        svg: SvgOutline.list
        popup: listFeedOptionsMenu
        onClicked: showFeed()
    }

    SkyMenuButton {
        text: isPinnedList ? qsTr("Remove favorite") : qsTr("Add favorite")
        svg: isPinnedList ? SvgFilled.star : SvgOutline.star
        svgColor: isPinnedList ? guiSettings.favoriteColor : guiSettings.textColor
        popup: listFeedOptionsMenu
        onClicked: {
            if (isPinnedList) {
                const favorite = skywalker.favoriteFeeds.getPinnedFeed(list.uri)

                if (favorite.listView.isNull()) {
                    console.warn("List is not pinned:", list.uri)
                    return
                }

                if (listFeedOptionsMenu.isOwnList())
                    skywalker.favoriteFeeds.removeList(favorite.listView) // We never show own lists as saved
                else
                    skywalker.favoriteFeeds.pinList(favorite.listView, false)

                isPinnedList = false
                skywalker.saveFavoriteFeeds()
            } else {
                graphUtils.getListView(list.uri)
            }
        }

        GraphUtils {
            id: graphUtils
            skywalker: listFeedOptionsMenu.skywalker

            onGetListOk: (userDid, list, viewPosts) => {
                console.debug("Pin List:", list.uri)
                skywalker.favoriteFeeds.pinList(list, true)
                isPinnedList = true
                skywalker.saveFavoriteFeeds()
            }

            onGetListFailed: (error) => skywalker.showStatusMessage(qsTr(`Failed to add favorite: ${error}`), QEnums.STATUS_LEVEL_ERROR)
        }
    }

    SkyMenuButton {
        id: hideListMenuItem
        visible: listFeedOptionsMenu.list?.purpose === QEnums.LIST_PURPOSE_CURATE && listFeedOptionsMenu.isOwnList()
        text: listFeedOptionsMenu.listHideFromTimeline ? qsTr("Unhide list from timeline") : qsTr("Hide list from timeline")
        svg: listFeedOptionsMenu.listHideFromTimeline ? SvgOutline.unmute : SvgOutline.mute
        popup: listFeedOptionsMenu
        onClicked: {
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

    SkyMenuButton {
        text: qsTr("Share")
        svg: SvgOutline.share
        popup: listFeedOptionsMenu
        onClicked: skywalker.getShareUtils().shareListUri(listFeedOptionsMenu.list.uri)
    }

    SkyMenuButton {
        text: qsTr("Copy list link")
        svg: SvgOutline.link
        popup: listFeedOptionsMenu
        onClicked: skywalker.getShareUtils().copyUriToClipboard(listFeedOptionsMenu.list.uri)
    }

    SkyMenuButton {
        text: qsTr("Filtered posts")
        svg: SvgOutline.hideVisibility
        popup: listFeedOptionsMenu
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

    function show(origin) {
        listHideFromTimeline = skywalker.getTimelineHide().hasList(list.uri)
        listHideReplies = userSettings.getFeedHideReplies(skywalker.getUserDid(), list.uri)
        listHideFollowing = userSettings.getFeedHideFollowing(skywalker.getUserDid(), list.uri)
        listSync = userSettings.mustSyncFeed(skywalker.getUserDid(), list.uri)
        x = origin.x
        y = origin.y
        open()
    }
}
