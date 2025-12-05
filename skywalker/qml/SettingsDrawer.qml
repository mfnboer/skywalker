import QtQuick
import QtQuick.Controls
import skywalker

SkyDrawer {
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
    signal activeFollows()
    signal signOut()
    signal about()
    signal buyCoffee()

    id: drawer
    width: Math.min(250 * guiSettings.fontScaleFactor, parent.width - 20)
    topPadding: guiSettings.headerMargin + 10
    bottomPadding: guiSettings.footerMargin + 10
    leftPadding: Math.max(20, guiSettings.leftMargin)
    rightPadding: Math.max(20, guiSettings.rightMargin)

    Flickable {
        anchors.fill: parent
        clip: true
        contentHeight: userColumn.height
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds

        Column {
            id: userColumn
            width: parent.width
            spacing: 5

            Column {
                width: parent.width
                spacing: 3

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

                AccessibleText {
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

                SkyMenu {
                    id: moderationMenu
                    onAboutToShow: enableSettingsPopupShield(true)
                    onAboutToHide: enableSettingsPopupShield(false)

                    CloseMenuItem {
                        text: qsTr("<b>Moderation</b>")
                        Accessible.name: qsTr("close moderation menu")
                    }
                    AccessibleMenuItem {
                        text: qsTr("Content Filtering")
                        svg: SvgOutline.visibility
                        onTriggered: contentFiltering()
                    }
                    AccessibleMenuItem {
                        text: qsTr("Blocked Accounts")
                        svg: SvgOutline.block
                        onTriggered: blockedAccounts()
                    }
                    AccessibleMenuItem {
                        text: qsTr("Muted Accounts")
                        svg: SvgOutline.mute
                        onTriggered: mutedAccounts()
                    }
                    AccessibleMenuItem {
                        text: qsTr("Muted Reposts")
                        svg: SvgOutline.repost
                        onTriggered: mutedReposts()
                    }
                    AccessibleMenuItem {
                        text: qsTr("Moderation Lists")
                        svg: SvgOutline.list
                        onTriggered: modLists()
                    }
                    AccessibleMenuItem {
                        text: qsTr("Muted Words")
                        svg: SvgOutline.mutedWords
                        onTriggered: mutedWords()
                    }
                    AccessibleMenuItem {
                        text: qsTr("Focus Hashtags")
                        svg: SvgOutline.hashtag
                        onTriggered: focusHashtags()
                    }
                }
            }

            SkyMenuItem {
                id: activeFollowsItem
                icon: SvgOutline.onlineUsers
                text: qsTr("Now Online")
                onClicked: activeFollows()
            }

            SkyMenuItem {
                id: userListsItem
                icon: SvgOutline.list
                text: qsTr("User Lists & Feeds")
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
                asynchronous: true

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
