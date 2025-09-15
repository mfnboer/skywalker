import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Rectangle {
    required property var timeline
    required property var skywalker
    property var searchView
    property int activePage: QEnums.UI_PAGE_NONE
    property bool floatingButtons: root.getSkywalker().getUserSettings().floatingNavigationButtons
    property int extraFooterMargin: 0
    property bool footerVisible: true

    signal homeClicked()
    signal notificationsClicked()
    signal searchClicked()
    signal messagesClicked()
    signal addConvoClicked()

    width: parent.width
    height: (visible && footerVisible) ? guiSettings.footerHeight : 0
    z: guiSettings.footerZLevel
    color: floatingButtons ? "transparent" : guiSettings.backgroundColor

    RowLayout {
        width: parent.width
        height: parent.height - guiSettings.footerMargin
        spacing: 0
        visible: footerVisible

        SkyFooterButton {
            Layout.preferredHeight: parent.height
            Layout.fillWidth: true
            floating: floatingButtons
            svg: isHomeActive() ? SvgFilled.home : SvgOutline.home
            counter: isHomeActive() && timeline ? timeline.unreadPosts : 0
            counterBackgroundColor: floatingButtons ? guiSettings.buttonNeutralColor : guiSettings.backgroundColor
            counterTextColor: guiSettings.textColor
            Accessible.name: getHomeSpeech()
            onClicked: homeClicked()
        }

        SkyFooterButton {
            Layout.preferredHeight: parent.height
            Layout.fillWidth: true
            floating: floatingButtons
            svg: isSearchActive() ? SvgFilled.search : SvgOutline.search
            Accessible.name: qsTr("search")
            onClicked: searchClicked()
        }

        Rectangle {
            Layout.preferredHeight: parent.height
            Layout.fillWidth: true
            color: "transparent"

            SvgButton {
                topInset: 0
                leftInset: 0
                rightInset: 0
                bottomInset: 0
                anchors.horizontalCenter: parent.horizontalCenter
                width: height
                height: parent.height
                svg: getSvg()
                accessibleName: qsTr("create post")

                onClicked: {
                    if (isMessagesActive())
                        addConvoClicked()
                    else
                        post()
                }

                function getSvg() {
                    if (isMessagesActive())
                        return SvgOutline.add

                    if (isHashtagSearch())
                        return SvgOutline.hashtag

                    return SvgOutline.chat
                }
            }
        }

        SkyFooterButton {
            Layout.preferredHeight: parent.height
            Layout.fillWidth: true
            floating: floatingButtons
            svg: isMessagesActive() ? SvgFilled.directMessage : SvgOutline.directMessage
            counter: skywalker.chat.unreadCount
            Accessible.name: skywalker.chat.unreadCount === 0 ? qsTr("direct messages") : qsTr(`${skywalker.chat.unreadCount} new direct messages`)
            onClicked: messagesClicked()
        }

        SkyFooterButton {
            Layout.preferredHeight: parent.height
            Layout.fillWidth: true
            floating: floatingButtons
            svg: isNotificationsActive() ? SvgFilled.notifications : SvgOutline.notifications
            counter: root.getSkywalker().unreadNotificationCount
            Accessible.name: root.getSkywalker().unreadNotificationCount === 0 ? qsTr("notifications") : qsTr(`${skywalker.unreadNotificationCount} new notifications`)
            onClicked: notificationsClicked()
        }
    }

    function isHashtagSearch() {
        if (!isSearchActive())
            return false

        return searchView.isHashtagSearch
    }

    function post() {
        if (isHashtagSearch())
            root.composePost("\n" + searchView.getSearchText())
        else
            root.composePost()
    }

    function getHomeSpeech() {
        if (!isHomeActive())
            return qsTr("show feed")

        if (!timeline)
            return ""

        if (timeline.unreadPosts === 0)
            return qsTr("You are at the top of your time line")

        if (timeline.unreadPosts === 1)
            return qsTr("1 post from the top of your time line")

        return qsTr(`${timeline.unreadPosts} posts from top of your timeline, press to go to top`)
    }

    function isHomeActive() {
        return activePage === QEnums.UI_PAGE_HOME
    }

    function isNotificationsActive() {
        return activePage === QEnums.UI_PAGE_NOTIFICATIONS
    }

    function isSearchActive() {
        return activePage === QEnums.UI_PAGE_SEARCH
    }

    function isMessagesActive() {
        return activePage === QEnums.UI_PAGE_MESSAGES
    }
}
