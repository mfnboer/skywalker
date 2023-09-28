import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Rectangle {
    required property var timeline
    required property var skywalker
    property bool homeActive: false
    property bool notificationsActive: false
    property int unreadNotifications: 0

    signal homeClicked()
    signal notificationsClicked()

    width: parent.width
    height: guiSettings.footerHeight
    z: guiSettings.footerZLevel
    color: guiSettings.footerColor

    RowLayout {
        width: parent.width
        height: parent.height

        Rectangle {
            height: parent.height
            Layout.fillWidth: true
            color: homeActive ? "transparent" : "lightgrey"

            SvgImage {
                id: homeButton
                y: height + 5
                width: height
                height: parent.height - 10
                anchors.horizontalCenter: parent.horizontalCenter
                color: guiSettings.textColor
                svg: svgOutline.home

                Rectangle {
                    x: parent.width - 17
                    y: -parent.y + 6
                    width: Math.max(unreadCountText.width + 10, height)
                    height: 20
                    radius: 8
                    color: guiSettings.badgeColor
                    border.color: homeActive ? guiSettings.badgeBorderColor : "lightgrey"
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
            }

            MouseArea {
                anchors.fill: parent
                onClicked: homeClicked()
            }
        }

        Rectangle {
            height: parent.height
            Layout.fillWidth: true
            color: notificationsActive ? "transparent" : "lightgrey"

            SvgImage {
                id: notificationsButton
                y: height + 5
                width: height
                height: parent.height - 10
                anchors.horizontalCenter: parent.horizontalCenter
                color: guiSettings.textColor
                svg: svgOutline.notifications

                Rectangle {
                    x: parent.width - 17
                    y: -parent.y + 6
                    width: Math.max(unreadNotificationsText.width + 10, height)
                    height: 20
                    radius: 8
                    color: guiSettings.badgeColor
                    border.color: notificationsActive ? guiSettings.badgeBorderColor : "lightgrey"
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

    SvgButton {
        x: parent.width - width - 10
        y: -height - 10
        width: 70
        height: width
        iconColor: guiSettings.buttonTextColor
        Material.background: guiSettings.buttonColor
        opacity: 0.6
        imageMargin: 20
        svg: svgOutline.chat
        onClicked: root.composePost()
    }
}
