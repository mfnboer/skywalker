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
    signal focusHashtags()
    signal settings()
    signal switchAccount()
    signal inviteCodes()
    signal bookmarks()
    signal signOut()
    signal about()
    signal buyCoffee()

    id: drawer
    width: Math.min(userColumn.width + 2 * padding, parent.width - 20)
    padding: 20

    Flickable {
        anchors.fill: parent
        clip: true
        contentHeight: userColumn.height
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds

        Column {
            id: userColumn
            width: Math.max(profileItem.width, moderationItem.width, settingsItem.width) + 70
            spacing: 5

            Column {
                width: parent.width
                spacing: 5

                Accessible.role: Accessible.StaticText
                Accessible.name: `${user.name} @${user.handle}`

                Avatar {
                    id: avatar
                    width: 60
                    height: width
                    author: user
                    onClicked: profile()

                    Accessible.ignored: true
                }

                SkyCleanedText {
                    id: nameText
                    width: parent.width
                    wrapMode: Text.Wrap
                    elide: Text.ElideRight
                    maximumLineCount: 2
                    font.bold: true
                    color: guiSettings.textColor
                    ellipsisBackgroundColor: Material.dialogColor
                    plainText: user.name

                    Accessible.ignored: true
                }

                Text {
                    id: handleText
                    width: parent.width
                    elide: Text.ElideRight
                    font.pointSize: guiSettings.scaledFont(7/8)
                    color: guiSettings.handleColor
                    text: `@${user.handle}`

                    Accessible.ignored: true
                }
            }

            SkyMenuItem {
                id: profileItem
                icon: svgOutline.user
                text: qsTr("Profile")
                onClicked: profile()
            }

            // Bluesky does not use invite codes anymore. New atproto providers may
            // SkyMenuItem {
            //     id: inviteCodesItem
            //     icon: svgOutline.inviteCode
            //     text: qsTr("Invite Codes") + ` (${inviteCodeCount})`
            //     onClicked: inviteCodes()
            // }

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

                    onAboutToShow: enableSettingsPopupShield(true)
                    onAboutToHide: enableSettingsPopupShield(false)

                    CloseMenuItem {
                        text: qsTr("<b>Moderation</b>")
                        Accessible.name: qsTr("close moderation menu")
                    }
                    AccessibleMenuItem {
                        text: qsTr("Content Filtering")
                        onTriggered: contentFiltering()

                        MenuItemSvg { svg: svgOutline.visibility }
                    }
                    AccessibleMenuItem {
                        text: qsTr("Blocked Accounts")
                        onTriggered: blockedAccounts()

                        MenuItemSvg { svg: svgOutline.block }
                    }
                    AccessibleMenuItem {
                        text: qsTr("Muted Accounts")
                        onTriggered: mutedAccounts()

                        MenuItemSvg { svg: svgOutline.mute }
                    }
                    AccessibleMenuItem {
                        text: qsTr("Muted Reposts")
                        onTriggered: mutedReposts()

                        MenuItemSvg { svg: svgOutline.repost }
                    }
                    AccessibleMenuItem {
                        text: qsTr("Moderation Lists")
                        onTriggered: modLists()

                        MenuItemSvg { svg: svgOutline.list }
                    }
                    AccessibleMenuItem {
                        text: qsTr("Muted Words")
                        onTriggered: mutedWords()

                        MenuItemSvg { svg: svgOutline.mutedWords }
                    }
                    AccessibleMenuItem {
                        text: qsTr("Focus Hashtags")
                        onTriggered: focusHashtags()

                        MenuItemSvg { svg: svgOutline.hashtag }
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

            Rectangle {
                width: 10
                height: width
                color: "transparent"
            }

            Image {
                id: buyMeCoffeeButton
                x: 30
                width: parent.width - 60
                fillMode: Image.PreserveAspectFit
                source: "/images/buycoffee.png"

                MouseArea {
                    anchors.fill: parent
                    onClicked: buyCoffee()
                }
            }
        }

        SvgButton {
            id: closeButton
            anchors.right: userColumn.right
            anchors.top: userColumn.top
            svg: svgOutline.close
            accessibleName: qsTr("close menu")
            onClicked: drawer.close()
        }
    }

    Rectangle {
        id: settingsPopupShield
        anchors.fill: parent
        color: "black"
        opacity: 0.2
        visible: false

        Accessible.role: Accessible.Window
    }

    GuiSettings {
        id: guiSettings
    }

    function enableSettingsPopupShield(enable) {
        settingsPopupShield.visible = enable
    }
}
