import QtQuick
import QtQuick.Controls.Material
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
    property int unreadNotifications: 0
    property bool showHomeFeedBadge: false

    signal homeClicked()
    signal notificationsClicked()
    signal searchClicked()
    signal feedsClicked()
    signal messagesClicked()
    signal addConvoClicked()

    width: parent.width
    height: guiSettings.footerHeight
    z: guiSettings.footerZLevel
    color: "transparent"

    // Shield to catch miss clicks.
    MouseArea {
        anchors.fill: parent
    }

    RowLayout {
        width: parent.width
        height: parent.height

        Rectangle {
            Layout.preferredHeight: parent.height
            Layout.fillWidth: true
            color: "transparent"

            SvgButton {
                id: homeButton
                y: 5
                width: height
                height: parent.height - 10
                anchors.horizontalCenter: parent.horizontalCenter
                Material.background: guiSettings.backgroundColor
                iconColor: guiSettings.textColor
                svg: homeActive ? SvgFilled.home : SvgOutline.home
                accessibleName: getHomeSpeech()
                onClicked: homeClicked()

                BadgeCounter {
                    color: guiSettings.backgroundColor
                    counterColor: guiSettings.textColor
                    counter: timeline.unreadPosts
                }

                // Feed badge
                Rectangle {
                    x: parent.width - 17
                    y: -parent.y + 6
                    width: 20
                    height: 20
                    radius: 10
                    color: guiSettings.backgroundColor
                    border.color: guiSettings.backgroundColor
                    border.width: 2
                    visible: showHomeFeedBadge

                    SkySvg {
                        x: 4
                        y: height + 2
                        width: 14
                        height: width
                        color: guiSettings.textColor
                        svg: SvgOutline.feed
                    }
                }
            }
        }

        Rectangle {
            Layout.preferredHeight: parent.height
            Layout.fillWidth: true
            color: "transparent"

            SvgButton {
                id: searchButton
                y: 5
                width: height
                height: parent.height - 10
                anchors.horizontalCenter: parent.horizontalCenter
                Material.background: guiSettings.backgroundColor
                iconColor: guiSettings.textColor
                svg: searchActive ? SvgFilled.search : SvgOutline.search
                accessibleName: qsTr("search")

                onClicked: searchClicked()
            }
        }

        Rectangle {
            Layout.preferredHeight: parent.height
            Layout.fillWidth: true
            color: "transparent"

            SvgButton {
                id: feedsButton
                y: 5
                width: height
                height: parent.height - 10
                anchors.horizontalCenter: parent.horizontalCenter
                Material.background: guiSettings.backgroundColor
                iconColor: guiSettings.textColor
                svg: feedsActive ? SvgFilled.feed : SvgOutline.feed
                accessibleName: qsTr("feeds")

                onClicked: feedsClicked()
            }
        }

        Rectangle {
            Layout.preferredHeight: parent.height
            Layout.fillWidth: true
            color: "transparent"

            SvgButton {
                id: messagesButton
                y: 5
                width: height
                height: parent.height - 10
                anchors.horizontalCenter: parent.horizontalCenter
                Material.background: guiSettings.backgroundColor
                iconColor: guiSettings.textColor
                svg: messagesActive ? SvgFilled.directMessage : SvgOutline.directMessage
                accessibleName: skywalker.chat.unreadCount === 0 ? qsTr("direct messages") : qsTr(`${skywalker.chat.unreadCount} new direct messages`)

                onClicked: messagesClicked()

                BadgeCounter {
                    counter: skywalker.chat.unreadCount
                }
            }
        }

        Rectangle {
            Layout.preferredHeight: parent.height
            Layout.fillWidth: true
            color: "transparent"

            SvgButton {
                id: notificationsButton
                y: 5
                width: height
                height: parent.height - 10
                anchors.horizontalCenter: parent.horizontalCenter
                Material.background: guiSettings.backgroundColor
                iconColor: guiSettings.textColor
                svg: notificationsActive ? SvgFilled.notifications : SvgOutline.notifications
                accessibleName: root.getSkywalker().unreadNotificationCount === 0 ? qsTr("notifications") : qsTr(`${skywalker.unreadNotificationCount} new notifications`)

                onClicked: notificationsClicked()

                BadgeCounter {
                    counter: root.getSkywalker().unreadNotificationCount
                }
            }
        }
    }

    PostButton {
        y: -height - 10
        svg: getSvg()
        overrideOnClicked: () => {
            if (messagesActive)
                addConvoClicked()
            else
                post()
        }

        Accessible.role: Accessible.Button
        Accessible.name: qsTr("post")
        Accessible.onPressAction: clicked()

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

        if (unreadPosts === 0)
            return qsTr("You are at the top of your time line")

        if (unreadPosts === 1)
            return qsTr("1 post from the top of your time line")

        return qsTr(`${timeline.unreadPosts} posts from top of your timeline, press to go to top`)
    }
}
