import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

Rectangle {
    property string userDid
    property Skywalker skywalker: root.getSkywalker(userDid)
    property bool reverseFeed: false
    required property string feedName
    property SvgImage defaultSvg: SvgFilled.feed
    property string feedAvatar
    property int contentMode: QEnums.CONTENT_MODE_UNSPECIFIED
    property int underlyingContentMode: QEnums.CONTENT_MODE_UNSPECIFIED
    property bool showAsHome: false
    property bool isHomeFeed: false
    property bool showLanguageFilter: false
    property list<language> filteredLanguages
    property bool showPostWithMissingLanguage: true
    property bool showMoreOptions: false
    property bool showViewOptions: false
    property bool showFavoritesPlaceHolder: false
    property bool isSideBar: false
    property int bottomMargin: 0
    readonly property int favoritesY: favoritesPlaceHolder.y

    signal closed
    signal feedAvatarClicked
    signal feedAvatarPressAndHold
    signal addUserView
    signal addHashtagView
    signal addFocusHashtagView
    signal addVideoView
    signal addMediaView
    signal filterStatistics
    signal viewChanged(int contentMode)

    id: header
    width: parent.width
    height: headerRow.y + headerRow.height + (showFavoritesPlaceHolder ? favoritesPlaceHolder.height : 0) + bottomMargin
    z: guiSettings.headerZLevel
    color: guiSettings.headerColor

    Accessible.role: Accessible.Pane

    RowLayout {
        id: headerRow
        width: parent.width
        height: header.visible ? (isSideBar ? guiSettings.sideBarHeaderHeight : guiSettings.headerHeight) : 0
        spacing: 0

        SvgPlainButton {
            id: backButton
            iconColor: guiSettings.headerTextColor
            svg: SvgOutline.arrowBack
            accessibleName: qsTr("go back")
            visible: !showAsHome

            onClicked: header.closed()
        }
        Loader {
            active: showMoreOptions

            sourceComponent: SvgPlainButton {
                id: moreButton
                iconColor: guiSettings.headerTextColor
                svg: SvgOutline.menu
                accessibleName: qsTr("more options")
                visible: showMoreOptions

                onClicked: moreMenu.open()

                SkyMenu {
                    id: moreMenu
                    menuWidth: 300

                    CloseMenuItem {
                        text: qsTr("<b>Options</b>")
                        Accessible.name: qsTr("close options menu")
                    }

                    AccessibleMenuItem {
                        text: qsTr("Add user view")
                        svg: SvgOutline.user
                        onTriggered: addUserView()
                    }

                    AccessibleMenuItem {
                        text: qsTr("Add hashtag view")
                        svg: SvgOutline.hashtag
                        onTriggered: addHashtagView()
                    }

                    AccessibleMenuItem {
                        id: focusMenuItem
                        text: qsTr("Add focus hashtag view")
                        svg: SvgOutline.hashtag
                        svgColor: guiSettings.favoriteColor
                        onTriggered: addFocusHashtagView()
                    }

                    AccessibleMenuItem {
                        text: qsTr("Add media view")
                        svg: SvgOutline.image
                        onTriggered: addMediaView()
                    }

                    AccessibleMenuItem {
                        text: qsTr("Add video view")
                        svg: SvgOutline.film
                        onTriggered: addVideoView()
                    }

                    AccessibleMenuItem {
                        text: qsTr("Filtered posts")
                        svg: SvgOutline.hideVisibility
                        onTriggered: filterStatistics()
                    }

                    PostsOrderMenu {
                        reverseFeed: header.reverseFeed
                        onReverseFeedChanged: header.reverseFeed = reverseFeed
                    }
                }
            }
        }
        FeedAvatar {
            Layout.leftMargin: 10
            Layout.rightMargin: 10
            Layout.preferredHeight: parent.height - 10
            Layout.preferredWidth: height
            userDid: header.userDid
            avatarUrl: header.feedAvatar
            contentMode: header.contentMode
            badgeOutlineColor: guiSettings.headerColor
            unknownSvg: defaultSvg
            visible: showAsHome && !isHomeFeed

            onClicked: header.feedAvatarClicked()
            onPressAndHold: header.feedAvatarPressAndHold()

            Accessible.role: Accessible.Button
            Accessible.name: header.feedName
            Accessible.onPressAction: clicked()
        }
        SkyCleanedTextLine {
            id: headerTexts
            Layout.fillWidth: true //!showAsHome
            Layout.alignment: Qt.AlignVCenter
            leftPadding: header.feedAvatar ? 0 : 10
            rightPadding: showAsHome ? expandFeedsButton.width : 0
            elide: Text.ElideRight
            font.bold: true
            font.pointSize: isSideBar ? guiSettings.scaledFont(1) : guiSettings.scaledFont(10/8)
            color: guiSettings.headerTextColor
            plainText: header.feedName

            Accessible.role: Accessible.ButtonDropDown
            Accessible.name: qsTr(`${header.feedName}, press to select other feed`)
            Accessible.description: Accessible.name
            Accessible.onPressAction: expandFeeds()

            ExpandFeedsButton {
                id: expandFeedsButton
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                skywalker: header.skywalker
                Layout.alignment: Qt.AlignLeft
                visible: showAsHome
            }

            SkyMouseArea {
                anchors.fill: parent
                onClicked: parent.expandFeeds()
            }

            function expandFeeds() {
                if (expandFeedsButton.visible)
                    expandFeedsButton.clicked() // qmllint disable missing-property
            }
        }

        SvgPlainButton {
            svg: SvgOutline.language
            iconColor: guiSettings.headerTextColor
            accessibleName: qsTr("language filter active")
            visible: showLanguageFilter && !isSideBar
            onClicked: showLanguageFilterDetails()
        }

        SvgPlainButton {
            svg: guiSettings.getContentModeSvg(contentMode)
            iconColor: guiSettings.headerTextColor
            accessibleName: qsTr("view mode")
            visible: showViewOptions && !isSideBar

            onClicked: viewMenuLoader.open()

            SkyMenuLoader {
                id: viewMenuLoader
                sourceComponent: viewMenuComponent
            }
        }

        FeedAvatar {
            Layout.rightMargin: 10
            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
            Layout.preferredHeight: parent.height - 10
            Layout.preferredWidth: Layout.preferredHeight
            userDid: header.userDid
            avatarUrl: header.feedAvatar
            contentMode: header.contentMode
            badgeOutlineColor: guiSettings.headerColor
            unknownSvg: defaultSvg
            visible: !showAsHome && !isHomeFeed && !isSideBar

            onClicked: header.feedAvatarClicked()

            Accessible.role: Accessible.Button
            Accessible.name: header.feedName
            Accessible.description: Accessible.name
            Accessible.onPressAction: header.feedAvatarClicked()
        }
        Loader {
            Layout.rightMargin: 10
            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
            Layout.preferredHeight: parent.height - 10
            Layout.preferredWidth: active ? Layout.preferredHeight : 0
            active: !root.isActiveUser(userDid)

            sourceComponent: CurrentUserAvatar {
                userDid: header.userDid
            }
        }
        Item {
            id: userAvatar
            Layout.rightMargin: 10
            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
            Layout.preferredHeight: parent.height - 10
            Layout.preferredWidth: Layout.preferredHeight
            visible: showAsHome && !isSideBar
            Accessible.role: Accessible.Pane

            Avatar {
                id: avatar
                width: parent.width
                author: skywalker.user
                onClicked: root.showSettingsDrawer()
                onPressAndHold: root.showSwitchUserDrawer()

                Accessible.role: Accessible.ButtonMenu
                Accessible.name: qsTr("Skywalker menu")
                Accessible.description: Accessible.name
                Accessible.onPressAction: clicked()
            }
        }
    }

    Loader {
        anchors.right: parent.right
        anchors.top: headerRow.bottom
        active: isSideBar

        sourceComponent: Column {
            spacing: 0

            SvgPlainButton {
                svg: guiSettings.getContentModeSvg(contentMode)
                iconColor: guiSettings.headerTextColor
                accessibleName: qsTr("view mode")
                visible: showViewOptions && isSideBar

                onClicked: viewMenuLoaderSideBar.open()

                SkyMenuLoader {
                    id: viewMenuLoaderSideBar
                    sourceComponent: viewMenuComponent
                }
            }

            SvgPlainButton {
                svg: SvgOutline.language
                iconColor: guiSettings.headerTextColor
                accessibleName: qsTr("language filter active")
                visible: showLanguageFilter && isSideBar
                onClicked: showLanguageFilterDetails()
            }
        }
    }

    Component {
        id: viewMenuComponent

        SkyMenu {
            id: viewMenu

            CloseMenuItem {
                text: qsTr("<b>View</b>")
                Accessible.name: qsTr("close view menu")
            }

            AccessibleMenuItem {
                text: qsTr("Post view")
                svg: SvgOutline.chat
                visible: underlyingContentMode === QEnums.CONTENT_MODE_UNSPECIFIED
                onTriggered: {
                    if (!isSideBar)
                        contentMode = QEnums.CONTENT_MODE_UNSPECIFIED

                    viewChanged(QEnums.CONTENT_MODE_UNSPECIFIED)
                }
            }
            AccessibleMenuItem {
                text: qsTr("Media view")
                svg: SvgOutline.image
                visible: underlyingContentMode === QEnums.CONTENT_MODE_UNSPECIFIED
                onTriggered: {
                    if (!isSideBar)
                        contentMode = QEnums.CONTENT_MODE_MEDIA

                    viewChanged(QEnums.CONTENT_MODE_MEDIA)
                }
            }
            AccessibleMenuItem {
                text: qsTr("Media gallery")
                svg: SvgOutline.gallery
                visible: underlyingContentMode === QEnums.CONTENT_MODE_UNSPECIFIED
                onTriggered: {
                    if (!isSideBar)
                        contentMode = QEnums.CONTENT_MODE_MEDIA_TILES

                    viewChanged(QEnums.CONTENT_MODE_MEDIA_TILES)
                }
            }
            AccessibleMenuItem {
                text: qsTr("Video view")
                svg: SvgOutline.film
                onTriggered: {
                    if (!isSideBar)
                        contentMode = QEnums.CONTENT_MODE_VIDEO

                    viewChanged(QEnums.CONTENT_MODE_VIDEO)
                }
            }
            AccessibleMenuItem {
                text: qsTr("Video gallery")
                svg: SvgOutline.videoGallery
                onTriggered: {
                    if (!isSideBar)
                        contentMode = QEnums.CONTENT_MODE_VIDEO_TILES

                    viewChanged(QEnums.CONTENT_MODE_VIDEO_TILES)
                }
            }
        }
    }

    Rectangle {
        id: favoritesPlaceHolder
        anchors.top: headerRow.bottom
        width: parent.width
        height: guiSettings.tabBarHeight
        color: guiSettings.backgroundColor
        visible: showFavoritesPlaceHolder
    }

    function showLanguageFilterDetails() {
        let languageList = []

        for (let i = 0; i < filteredLanguages.length; ++i) {
            const lang = `${filteredLanguages[i].nativeName} (${filteredLanguages[i].shortCode})`
            languageList.push(lang)
            console.debug(lang)
        }

        let msg = qsTr("Language filter is active.") + "<br><br>"

        if (languageList.length > 0) {
            msg += qsTr("Only posts with the following languages will be shown:") +
                    " " + languageList.join(", ") + "<br><br>"
        }

        if (!showPostWithMissingLanguage)
            msg += qsTr("Posts without language tags will not be shown.")
        else
            msg += qsTr("Posts without language tags will be shown.")

        guiSettings.notice(rootContent, msg)
    }
}
