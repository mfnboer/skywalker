import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Drawer {
    property basicprofile user
    property int inviteCodeCount: 0
    readonly property int iconSize: 30
    readonly property real menuFontSize: guiSettings.scaledFont(10/8)

    signal profile()
    signal contentFiltering()
    signal blockedAccounts()
    signal mutedAccounts()
    signal mutedReposts()
    signal modLists()
    signal userLists()
    signal mutedWords()
    signal settings()
    signal switchAccount()
    signal inviteCodes()
    signal bookmarks()
    signal signOut()
    signal about()

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
            color: guiSettings.textColor
            text: user.name
        }

        Text {
            elide: Text.ElideRight
            font.pointSize: guiSettings.scaledFont(7/8)
            color: guiSettings.handleColor
            text: `@${user.handle}`
        }

        Rectangle {
            width: 20
            height: width
            color: "transparent"
        }

        SkyMenuItem {
            id: profileItem
            icon: svgOutline.user
            text: qsTr("Profile")
            onClicked: profile()
        }

        SkyMenuItem {
            id: inviteCodesItem
            icon: svgOutline.inviteCode
            text: qsTr("Invite Codes") + ` (${inviteCodeCount})`
            onClicked: inviteCodes()
        }

        SkyMenuItem {
            id: bookmarksItem
            icon: svgOutline.bookmark
            text: qsTr("Bookmarks")
            onClicked: bookmarks()
        }

        SkyMenuItem {
            id: moderationItem
            icon: svgOutline.moderation
            text: qsTr("Moderation")
            onClicked: moderationMenu.open()

            Menu {
                id: moderationMenu
                modal: true

                MenuItem {
                    text: qsTr("Content Filtering")
                    onTriggered: contentFiltering()

                    MenuItemSvg {
                        svg: svgOutline.visibility
                    }
                }
                MenuItem {
                    text: qsTr("Blocked Accounts")
                    onTriggered: blockedAccounts()

                    MenuItemSvg {
                        svg: svgOutline.block
                    }
                }
                MenuItem {
                    text: qsTr("Muted Accounts")
                    onTriggered: mutedAccounts()

                    MenuItemSvg {
                        svg: svgOutline.mute
                    }
                }
                MenuItem {
                    text: qsTr("Muted Reposts")
                    onTriggered: mutedReposts()

                    MenuItemSvg {
                        svg: svgOutline.repost
                    }
                }
                MenuItem {
                    text: qsTr("Moderation Lists")
                    onTriggered: modLists()

                    MenuItemSvg {
                        svg: svgOutline.list
                    }
                }
                MenuItem {
                    text: qsTr("Muted Words")
                    onTriggered: mutedWords()

                    MenuItemSvg {
                        svg: svgOutline.mutedWords
                    }
                }
            }
        }

        SkyMenuItem {
            id: userListsItem
            icon: svgOutline.list
            text: qsTr("User Lists")
            onClicked: userLists()
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

        SkyMenuItem {
            id: aboutItem
            icon: svgOutline.info
            text: qsTr("About")
            onClicked: about()
        }
    }

    GuiSettings {
        id: guiSettings
    }
}
