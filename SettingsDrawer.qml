import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Drawer {
    property basicprofile user
    readonly property int iconSize: 30
    readonly property real menuFontSize: guiSettings.scaledFont(10/8)

    signal profile()
    signal contentFiltering()
    signal blockedAccounts()
    signal mutedAccounts()
    signal settings()
    signal switchAccount()
    signal signOut()

    width: Math.min(userColumn.width + 2 * padding, parent.width - 20)
    padding: 20

    Column {
        id: userColumn
        anchors.top: parent.top
        width: Math.max(profileItem.width, moderationItem.width, settingsItem.width) + 70
        spacing: 5

        Avatar {
            width: 60
            height: width
            avatarUrl: user.avatarUrl
            onClicked: profile()
        }

        Text {
            wrapMode: Text.Wrap
            elide: Text.ElideRight
            maximumLineCount: 2
            font.bold: true
            text: user.name
        }

        Text {
            elide: Text.ElideRight
            font.pointSize: guiSettings.scaledFont(7/8)
            color: guiSettings.handleColor
            text: `@${user.handle}`
        }

        Rectangle {
            width: 30
            height: width
        }

        SkyMenuItem {
            id: profileItem
            icon: svgOutline.user
            text: qsTr("Profile")
            onClicked: profile()
        }

        SkyMenuItem {
            id: moderationItem
            icon: svgOutline.visibility
            text: qsTr("Moderation")
            onClicked: moderationMenu.open()

            Menu {
                id: moderationMenu

                MenuItem {
                    text: qsTr("Content Filtering")
                    onTriggered: contentFiltering()
                }
                MenuItem {
                    text: qsTr("Blocked Accounts")
                    onTriggered: blockedAccounts()
                }
                MenuItem {
                    text: qsTr("Muted Accounts")
                    onTriggered: mutedAccounts()
                }
            }
        }

        SkyMenuItem {
            id: settingsItem
            icon: svgOutline.settings
            text: qsTr("Settings")
            onClicked: settings()
        }

        SkyMenuItem {
            id: switchAccountItem
            icon: svgOutline.group
            text: qsTr("Switch Account")
            onClicked: switchAccount()
        }

        SkyMenuItem {
            id: signOutItem
            icon: svgOutline.signOut
            text: qsTr("Sign Out")
            onClicked: signOut()
        }
    }

    GuiSettings {
        id: guiSettings
    }
}
