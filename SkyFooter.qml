import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Rectangle {
    required property var timeline

    signal homeClicked()
    signal notificationsClicked()

    width: parent.width
    height: guiSettings.footerHeight
    z: guiSettings.footerZLevel
    color: guiSettings.footerColor

    Row {
        width: parent.width

        SvgImage {
            id: homeButton
            y: height + 5
            width: height
            height: guiSettings.footerHeight - 10
            Layout.fillWidth: true
            color: guiSettings.textColor
            svg: svgOutline.home

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

            MouseArea {
                y: -parent.y
                width: parent.width
                height: parent.height
                onClicked: homeClicked()
            }
        }

        SvgImage {
            id: notificationsButton
            y: height + 5
            width: height
            height: guiSettings.footerHeight - 10
            Layout.fillWidth: true
            color: guiSettings.textColor
            svg: svgOutline.notifications

            Rectangle {
                x: parent.width - 17
                y: -parent.y + 6
                width: Math.max(unreadNotificationsText.width + 10, height)
                height: 20
                radius: 8
                color: guiSettings.badgeColor
                border.color: guiSettings.badgeBorderColor
                border.width: 2
                visible: false // TODO

                Text {
                    id: unreadNotificationsText
                    anchors.centerIn: parent
                    font.bold: true
                    font.pointSize: guiSettings.scaledFont(6/8)
                    color: guiSettings.badgeTextColor
                    text: "0" // TODO
                }
            }

            MouseArea {
                y: -parent.y
                width: parent.width
                height: parent.height
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
