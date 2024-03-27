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
    property int unreadNotifications: 0
    property bool showHomeFeedBadge: false

    signal homeClicked()
    signal notificationsClicked()
    signal searchClicked()
    signal feedsClicked()

    width: parent.width
    height: guiSettings.footerHeight
    z: guiSettings.footerZLevel
    color: guiSettings.footerColor

    Rectangle {
        id: separatorLine
        width: parent.width
        height: 1
        color: guiSettings.separatorColor
        Accessible.ignored: true
    }

    RowLayout {
        anchors.top: separatorLine.bottom
        width: parent.width
        height: parent.height - separatorLine.height

        Rectangle {
            height: parent.height
            Layout.fillWidth: true
            color: guiSettings.backgroundColor

            Accessible.role: Accessible.PageTab
            Accessible.name: getHomeSpeech()
            Accessible.onPressAction: homeClicked()

            SvgImage {
                id: homeButton
                y: height + 5
                width: height
                height: parent.height - 10
                anchors.horizontalCenter: parent.horizontalCenter
                color: guiSettings.textColor
                svg: homeActive ? svgFilled.home : svgOutline.home

                // Badge counter
                Rectangle {
                    x: parent.width - 17
                    y: -parent.y + 6
                    width: Math.max(unreadCountText.width + 10, height)
                    height: 20
                    radius: 8
                    color: guiSettings.badgeColor
                    border.color: guiSettings.badgeBorderColor
                    border.width: 2
                    visible: timeline.unreadPosts > 0

                    Text {
                        id: unreadCountText
                        anchors.centerIn: parent
                        font.bold: true
                        font.pointSize: guiSettings.scaledFont(6/8)
                        color: guiSettings.badgeTextColor
                        text: timeline.unreadPosts
                    }
                }

                // Feed badge
                Rectangle {
                    x: parent.width - 17
                    y: -parent.y + 6
                    width: 20
                    height: 20
                    radius: 10
                    color: guiSettings.textColor
                    border.color: guiSettings.backgroundColor
                    border.width: 2
                    visible: showHomeFeedBadge

                    SvgImage {
                        x: 4
                        y: height + 2
                        width: 14
                        height: width
                        color: guiSettings.backgroundColor
                        svg: svgOutline.feed
                    }
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: homeClicked()
            }
        }

        Rectangle {
            height: parent.height
            Layout.fillWidth: true
            color: guiSettings.backgroundColor

            Accessible.role: Accessible.PageTab
            Accessible.name: qsTr("search")
            Accessible.onPressAction: searchClicked()

            SvgImage {
                id: searchButton
                y: height + 5
                width: height
                height: parent.height - 10
                anchors.horizontalCenter: parent.horizontalCenter
                color: guiSettings.textColor
                svg: searchActive ? svgFilled.search : svgOutline.search
            }

            MouseArea {
                anchors.fill: parent
                onClicked: searchClicked()
            }
        }

        Rectangle {
            height: parent.height
            Layout.fillWidth: true
            color: guiSettings.backgroundColor

            Accessible.role: Accessible.PageTab
            Accessible.name: qsTr("feeds")
            Accessible.onPressAction: feedsClicked()

            SvgImage {
                id: feedsButton
                y: height + 5
                width: height
                height: parent.height - 10
                anchors.horizontalCenter: parent.horizontalCenter
                color: guiSettings.textColor
                svg: feedsActive ? svgFilled.feed : svgOutline.feed
            }

            MouseArea {
                anchors.fill: parent
                onClicked: feedsClicked()
            }
        }

        Rectangle {
            height: parent.height
            Layout.fillWidth: true
            color: guiSettings.backgroundColor

            Accessible.role: Accessible.PageTab
            Accessible.name: skywalker.unreadNotificationCount === 0 ? qsTr("notifications") : qsTr(`${skywalker.unreadNotificationCount} new notifications`)
            Accessible.onPressAction: notificationsClicked()

            SvgImage {
                id: notificationsButton
                y: height + 5
                width: height
                height: parent.height - 10
                anchors.horizontalCenter: parent.horizontalCenter
                color: guiSettings.textColor
                svg: notificationsActive ? svgFilled.notifications : svgOutline.notifications
                Accessible.ignored: true

                Rectangle {
                    x: parent.width - 17
                    y: -parent.y + 6
                    width: Math.max(unreadNotificationsText.width + 10, height)
                    height: 20
                    radius: 8
                    color: guiSettings.badgeColor
                    border.color: guiSettings.badgeBorderColor
                    border.width: 2
                    visible: skywalker.unreadNotificationCount > 0

                    Text {
                        id: unreadNotificationsText
                        anchors.centerIn: parent
                        font.bold: true
                        font.pointSize: guiSettings.scaledFont(6/8)
                        color: guiSettings.badgeTextColor
                        text: skywalker.unreadNotificationCount
                    }
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: notificationsClicked()
            }
        }
    }

    PostButton {
        y: -height - 10
        svg: isHashtagSearch() ? svgOutline.hashtag : svgOutline.chat
        overrideOnClicked: () => post()

        Accessible.role: Accessible.Button
        Accessible.name: qsTr("post")
        Accessible.onPressAction: clicked()
    }

    GuiSettings {
        id: guiSettings
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
