import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Rectangle {
    required property var skywalker
    required property string feedName
    property string feedAvatar
    property bool showAsHome: false
    property bool isHomeFeed: false

    signal closed
    signal feedAvatarClicked

    id: header
    width: parent.width
    height: guiSettings.headerHeight
    z: guiSettings.headerZLevel
    color: guiSettings.headerColor

    RowLayout {
        id: headerRow
        width: parent.width
        height: guiSettings.headerHeight

        SvgButton {
            id: backButton
            iconColor: guiSettings.headerTextColor
            Material.background: "transparent"
            svg: svgOutline.arrowBack
            visible: !showAsHome

            onClicked: header.closed()
        }
        FeedAvatar {
            Layout.leftMargin: 10
            Layout.rightMargin: 10
            height: parent.height - 10
            width: height
            avatarUrl: header.feedAvatar
            visible: showAsHome && !isHomeFeed

            onClicked: header.feedAvatarClicked()
        }
        Text {
            id: headerTexts
            Layout.fillWidth: !showAsHome
            Layout.alignment: Qt.AlignVCenter
            leftPadding: header.feedAvatar ? 0 : 10
            font.bold: true
            font.pointSize: guiSettings.scaledFont(10/8)
            color: guiSettings.headerTextColor
            text: header.feedName

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (expandFeedsButton.visible)
                        expandFeedsButton.onClicked()
                }
            }
        }
        ExpandFeedsButton {
            id: expandFeedsButton
            skywalker: header.skywalker
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignLeft
            visible: showAsHome
        }
        FeedAvatar {
            Layout.rightMargin: 10
            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
            height: parent.height - 10
            width: height
            avatarUrl: header.feedAvatar
            visible: !showAsHome && !isHomeFeed

            onClicked: header.feedAvatarClicked()
        }
        Item {
            Layout.rightMargin: 10
            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
            height: parent.height - 10
            width: height
            visible: showAsHome

            Avatar {
                id: avatar
                width: parent.width
                height: parent.height
                avatarUrl: skywalker.avatarUrl
                onClicked: root.showSettingsDrawer()
                onPressAndHold: root.showSwitchUserDrawer()
            }
        }
    }

    GuiSettings {
        id: guiSettings
    }
}
