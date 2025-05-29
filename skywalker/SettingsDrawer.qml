import QtQuick
import QtQuick.Controls
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
    width: Math.min(userColumn.width + leftPadding + rightPadding, parent.width - 20)
    topPadding: guiSettings.headerMargin + 10
    bottomPadding: guiSettings.footerMargin + 10
    leftPadding: 20
    rightPadding: 20

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
                    author: user
                    onClicked: profile()

                    Accessible.ignored: true
                }

                AuthorNameAndStatusMultiLine {
                    id: nameText
                    width: parent.width
                    wrapMode: Text.Wrap
                    maximumLineCount: 2
                    ellipsisBackgroundColor: Material.dialogColor
                    author: user
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
                icon: SvgOutline.user
                text: qsTr("Profile")
                onClicked: profile()
            }

            // Bluesky does not use invite codes anymore. New atproto providers may
            // SkyMenuItem {
            //     id: inviteCodesItem
            //     icon: SvgOutline.inviteCode
            //     text: qsTr("Invite Codes") + ` (${inviteCodeCount})`
            //     onClicked: inviteCodes()
            // }

            SkyMenuItem {
                id: bookmarksItem
                icon: SvgOutline.bookmark
                text: qsTr("Bookmarks")
                onClicked: bookmarks()
            }

            SkyMenuItem {
                id: moderationItem
                icon: SvgOutline.moderation
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

                        MenuItemSvg { svg: SvgOutline.visibility }
                    }
                    AccessibleMenuItem {
                        text: qsTr("Blocked Accounts")
                        onTriggered: blockedAccounts()

                        MenuItemSvg { svg: SvgOutline.block }
                    }
                    AccessibleMenuItem {
                        text: qsTr("Muted Accounts")
                        onTriggered: mutedAccounts()

                        MenuItemSvg { svg: SvgOutline.mute }
                    }
                    AccessibleMenuItem {
                        text: qsTr("Muted Reposts")
                        onTriggered: mutedReposts()

                        MenuItemSvg { svg: SvgOutline.repost }
                    }
                    AccessibleMenuItem {
                        text: qsTr("Moderation Lists")
                        onTriggered: modLists()

                        MenuItemSvg { svg: SvgOutline.list }
                    }
                    AccessibleMenuItem {
                        text: qsTr("Muted Words")
                        onTriggered: mutedWords()

                        MenuItemSvg { svg: SvgOutline.mutedWords }
                    }
                    AccessibleMenuItem {
                        text: qsTr("Focus Hashtags")
                        onTriggered: focusHashtags()

                        MenuItemSvg { svg: SvgOutline.hashtag }
                    }
                }
            }

            SkyMenuItem {
                id: userListsItem
                icon: SvgOutline.list
                text: qsTr("User Lists")
                onClicked: userLists()
            }

            SkyMenuItem {
                id: settingsItem
                icon: SvgOutline.settings
                text: qsTr("Settings")
                onClicked: settings()
            }

            SkyMenuItem {
                id: switchAccountItem
                icon: SvgOutline.group
                text: qsTr("Switch Account")
                onClicked: switchAccount()
            }

            SkyMenuItem {
                id: signOutItem
                icon: SvgOutline.signOut
                text: qsTr("Sign Out")
                onClicked: signOut()
            }

            SkyMenuItem {
                id: aboutItem
                icon: SvgOutline.info
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
            svg: SvgOutline.close
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


    function enableSettingsPopupShield(enable) {
        settingsPopupShield.visible = enable
    }
}
