import QtQuick
import QtQuick.Controls
import skywalker

SkyDrawer {
    property basicprofile user
    property int inviteCodeCount: 0
    property Skywalker skywalker: root.getSkywalker()
    property UserSettings userSettings: skywalker.getUserSettings();
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
    signal verifiers()
    signal settings()
    signal switchAccount()
    signal inviteCodes()
    signal bookmarks()
    signal activeFollows()
    signal backup()
    signal restore()
    signal backupHelp()
    signal signOut()
    signal about()
    signal buyCoffee()

    id: drawer
    width: guiSettings.scaleWidthToFont(250)
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
            spacing: 10

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

                AuthorNameAndStatus {
                    id: nameText
                    width: parent.width
                    wrapMode: Text.Wrap
                    maximumLineCount: 2
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

                    SkyMenuButton {
                        text: qsTr("Content Filtering")
                        svg: SvgOutline.visibility
                        popup: moderationMenu
                        onClicked: contentFiltering()
                    }
                    SkyMenuButton {
                        text: qsTr("Blocked Accounts")
                        svg: SvgOutline.block
                        popup: moderationMenu
                        onClicked: blockedAccounts()
                    }
                    SkyMenuButton {
                        text: qsTr("Muted Accounts")
                        svg: SvgOutline.mute
                        popup: moderationMenu
                        onClicked: mutedAccounts()
                    }
                    SkyMenuButton {
                        text: qsTr("Muted Reposts")
                        svg: SvgOutline.repost
                        popup: moderationMenu
                        onClicked: mutedReposts()
                    }
                    SkyMenuButton {
                        text: qsTr("Moderation Lists")
                        svg: SvgOutline.list
                        popup: moderationMenu
                        onClicked: modLists()
                    }
                    SkyMenuButton {
                        text: qsTr("Muted Words")
                        svg: SvgOutline.mutedWords
                        popup: moderationMenu
                        onClicked: mutedWords()
                    }
                    SkyMenuButton {
                        text: qsTr("Focus Hashtags")
                        svg: SvgOutline.focusHashtag
                        popup: moderationMenu
                        onClicked: focusHashtags()
                    }
                    SkyMenuButton {
                        text: qsTr("Trusted Verifiers")
                        svg: SvgOutline.verifier
                        popup: moderationMenu
                        onClicked: verifiers()
                    }
                    MenuSeparator {}
                    AccessibleMenuItem {
                        text: qsTr("Track filtered posts")
                        checkable: true
                        checked: userSettings.contentFilterStatsEnabled
                        onCheckedChanged: userSettings.contentFilterStatsEnabled = checked
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
                icon: SvgOutline.group
                text: qsTr("Accounts")
                onClicked: accountsMenu.open()

                SkyMenu {
                    id: accountsMenu

                    SkyMenuButton {
                        svg: SvgOutline.group
                        text: qsTr("Switch Account")
                        popup: accountsMenu
                        onClicked: switchAccount()
                    }
                    SkyMenuButton {
                        svg: SvgOutline.signOut
                        text: qsTr("Sign Out")
                        popup: accountsMenu
                        onClicked: signOut()
                    }
                }
            }

            SkyMenuItem {
                id: backupItem
                icon: SvgOutline.save
                text: qsTr("Backup")
                onClicked: backupMenu.open()

                SkyMenu {
                    id: backupMenu

                    SkyMenuButton {
                        text: qsTr("Backup settings")
                        svg: SvgOutline.upload
                        popup: backupMenu
                        onClicked: backup()
                    }
                    SkyMenuButton {
                        text: qsTr("Restore settings")
                        svg: SvgOutline.download
                        popup: backupMenu
                        onClicked: restore()
                    }
                    SkyMenuButton {
                        text: qsTr("Help")
                        svg: SvgOutline.help
                        popup: backupMenu
                        onClicked: backupHelp()
                    }
                }
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

                SkyMouseArea {
                    anchors.fill: parent
                    onClicked: buyCoffee()
                }
            }
        }
    }
}
