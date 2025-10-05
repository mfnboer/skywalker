import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Rectangle {
    required property var timeline
    required property var skywalker
    property var searchView
    property int activePage: QEnums.UI_PAGE_NONE
    property int extraFooterMargin: 0
    property bool footerVisible: true

    signal homeClicked()
    signal notificationsClicked()
    signal searchClicked()
    signal messagesClicked()
    signal addConvoClicked()

    id: footer
    width: parent.width
    height: (visible && footerVisible) ? guiSettings.footerHeight : 0
    z: guiSettings.footerZLevel
    color: guiSettings.backgroundColor

    RowLayout {
        width: parent.width
        height: parent.height - guiSettings.footerMargin
        spacing: 0
        visible: footerVisible

        SkyFooterButton {
            Layout.preferredHeight: parent.height
            Layout.fillWidth: true
            svg: isHomeActive() ? SvgFilled.home : SvgOutline.home
            counter: isHomeActive() && timeline ? timeline.unreadPosts : 0
            counterBackgroundColor: guiSettings.backgroundColor
            counterTextColor: guiSettings.textColor
            Accessible.name: getHomeSpeech()
            onClicked: homeClicked()
        }

        SkyFooterButton {
            Layout.preferredHeight: parent.height
            Layout.fillWidth: true
            svg: isSearchActive() ? SvgFilled.search : SvgOutline.search
            Accessible.name: qsTr("search")
            onClicked: searchClicked()
        }

        Rectangle {
            Layout.preferredHeight: parent.height
            Layout.fillWidth: true
            color: "transparent"

            FooterPostButton {
                messagesActive: isMessagesActive()
                hashtagSearch: isHashtagSearch()
                searchView: footer.searchView

                anchors.horizontalCenter: parent.horizontalCenter
                width: height
                height: parent.height

                onAddConvoClicked: footer.addConvoClicked()
            }
        }

        SkyFooterButton {
            Layout.preferredHeight: parent.height
            Layout.fillWidth: true
            svg: isMessagesActive() ? SvgFilled.directMessage : SvgOutline.directMessage
            counter: skywalker.chat.unreadCount
            Accessible.name: skywalker.chat.unreadCount === 0 ? qsTr("direct messages") : qsTr(`${skywalker.chat.unreadCount} new direct messages`)
            onClicked: messagesClicked()
        }

        SkyFooterButton {
            Layout.preferredHeight: parent.height
            Layout.fillWidth: true
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
