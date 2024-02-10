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
            Accessible.name: homeActive ?
                                 qsTr(`${timeline.unreadPosts} posts from top of feed, press to go to top`) :
                                 qsTr("show feed")
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
            Accessible.onPressAction: searchClicked()

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
        x: parent.width - width - 10
        y: -height - 10

        Accessible.role: Accessible.Button
        Accessible.name: qsTr("post")
        Accessible.onPressAction: clicked()
    }

    GuiSettings {
        id: guiSettings
    }
}
