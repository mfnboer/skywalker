import QtQuick
import QtQuick.Controls
import skywalker

SkyMenu {
    required property bool reverseFeed
    property Skywalker skywalker: root.getSkywalker()
    property UserSettings userSettings: skywalker.getUserSettings()

    signal addUserView
    signal addHashtagView
    signal addFocusHashtagView
    signal addVideoView
    signal addMediaView
    signal filterStatistics
    signal newReverseFeed(bool reverse)

    id: moreMenu
    menuWidth: 250

    SkyMenuButton {
        text: qsTr("Add user view")
        svg: SvgOutline.user
        popup: moreMenu
        onClicked: addUserView()
    }

    SkyMenuButton {
        text: qsTr("Add hashtag view")
        svg: SvgOutline.hashtag
        popup: moreMenu
        onClicked: addHashtagView()
    }

    SkyMenuButton {
        text: qsTr("Add focus hashtag view")
        svg: SvgOutline.focusHashtag
        popup: moreMenu
        onClicked: addFocusHashtagView()
    }

    SkyMenuButton {
        text: qsTr("Add media view")
        svg: SvgOutline.image
        popup: moreMenu
        onClicked: addMediaView()
    }

    SkyMenuButton {
        text: qsTr("Add video view")
        svg: SvgOutline.film
        popup: moreMenu
        onClicked: addVideoView()
    }

    SkyMenuButton {
        text: qsTr("Filtered posts")
        svg: SvgOutline.hideVisibility
        popup: moreMenu
        onClicked: filterStatistics()
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
        checked: !reverseFeed
        enabled: userSettings.globalFeedOrder === QEnums.FEED_ORDER_PER_FEED
        onTriggered: newReverseFeed(false)
    }

    SkyRadioMenuItem {
        text: qsTr("Old to new")
        checked: reverseFeed
        enabled: userSettings.globalFeedOrder === QEnums.FEED_ORDER_PER_FEED
        onTriggered: newReverseFeed(true)
    }
}
