import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Rectangle {
    required property var timeline
    required property var skywalker
    property bool homeActive: false
    property bool notificationsActive: false
    property bool searchActive: false
    property bool feedsActive: false
    property bool messagesActive: false
    property bool showHomeFeedBadge: false
    property bool floatingButtons: root.getSkywalker().getUserSettings().floatingNavigationButtons

    signal homeClicked()
    signal notificationsClicked()
    signal searchClicked()
    signal feedsClicked()
    signal messagesClicked()
    signal addConvoClicked()

    width: parent.width
    height: guiSettings.footerHeight
    z: guiSettings.footerZLevel
    color: floatingButtons ? "transparent" : guiSettings.backgroundColor

    RowLayout {
        width: parent.width
        height: parent.height
        spacing: 0

        SkyFooterButton {
            Layout.preferredHeight: parent.height
            Layout.fillWidth: true
            floating: floatingButtons
            svg: homeActive ? SvgFilled.home : SvgOutline.home
            counter: homeActive ? timeline.unreadPosts : 0
            counterBackgroundColor: floatingButtons ? guiSettings.buttonNeutralColor : guiSettings.backgroundColor
            counterTextColor: guiSettings.textColor
            showAltBadge: showHomeFeedBadge
            altBadgeSvg: SvgOutline.feed
            Accessible.name: getHomeSpeech()
            onClicked: homeClicked()
        }

        SkyFooterButton {
            Layout.preferredHeight: parent.height
            Layout.fillWidth: true
            floating: floatingButtons
            svg: searchActive ? SvgFilled.search : SvgOutline.search
            Accessible.name: qsTr("search")
            onClicked: searchClicked()
        }

        SkyFooterButton {
            Layout.preferredHeight: parent.height
            Layout.fillWidth: true
            floating: floatingButtons
            svg: feedsActive ? SvgFilled.feed : SvgOutline.feed
            Accessible.name: qsTr("feeds")
            onClicked: feedsClicked()
        }

        SkyFooterButton {
            Layout.preferredHeight: parent.height
            Layout.fillWidth: true
            floating: floatingButtons
            svg: messagesActive ? SvgFilled.directMessage : SvgOutline.directMessage
            counter: skywalker.chat.unreadCount
            Accessible.name: skywalker.chat.unreadCount === 0 ? qsTr("direct messages") : qsTr(`${skywalker.chat.unreadCount} new direct messages`)
            onClicked: messagesClicked()
        }

        SkyFooterButton {
            Layout.preferredHeight: parent.height
            Layout.fillWidth: true
            floating: floatingButtons
            svg: notificationsActive ? SvgFilled.notifications : SvgOutline.notifications
            counter: root.getSkywalker().unreadNotificationCount
            Accessible.name: root.getSkywalker().unreadNotificationCount === 0 ? qsTr("notifications") : qsTr(`${skywalker.unreadNotificationCount} new notifications`)
            onClicked: notificationsClicked()
        }
    }

    PostButton {
        y: -height - 10
        svg: getSvg()
        accessibleName: qsTr("post")
        overrideOnClicked: () => {
            if (messagesActive)
                addConvoClicked()
            else
                post()
        }

        function getSvg() {
            if (messagesActive)
                return SvgOutline.add

            if (isHashtagSearch())
                return SvgOutline.hashtag

            return SvgOutline.chat
        }
    }

    function isHashtagSearch() {
        if (!searchActive)
            return false

        return parent.isHashtagSearch
    }

    function post() {
        if (isHashtagSearch())
            root.composePost("\n" + parent.getSearchText())
        else
            root.composePost()
    }

    function getHomeSpeech() {
        if (!homeActive)
            return qsTr("show feed")

        if (timeline.unreadPosts === 0)
            return qsTr("You are at the top of your time line")

        if (timeline.unreadPosts === 1)
            return qsTr("1 post from the top of your time line")

        return qsTr(`${timeline.unreadPosts} posts from top of your timeline, press to go to top`)
    }
}
