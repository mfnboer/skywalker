import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

Rectangle {
    required property var skywalker
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
    property int bottomMargin: 0
    readonly property int favoritesY: favoritesPlaceHolder.y

    signal closed
    signal feedAvatarClicked
    signal addUserView
    signal addHashtagView
    signal addFocusHashtagView
    signal addVideoView
    signal addMediaView
    signal viewChanged(int contentMode)

    id: header
    width: parent.width
    height: headerRow.height + (showFavoritesPlaceHolder ? favoritesPlaceHolder.height : 0) + bottomMargin
    z: guiSettings.headerZLevel
    color: guiSettings.headerColor

    Accessible.role: Accessible.Pane

    RowLayout {
        id: headerRow
        width: parent.width
        height: guiSettings.headerHeight
        spacing: 0

        SvgButton {
            id: backButton
            iconColor: guiSettings.headerTextColor
            Material.background: "transparent"
            svg: SvgOutline.arrowBack
            accessibleName: qsTr("go back")
            visible: !showAsHome

            onClicked: header.closed()
        }
        SvgButton {
            id: moreButton
            iconColor: guiSettings.headerTextColor
            Material.background: "transparent"
            svg: SvgOutline.menu
            accessibleName: qsTr("more options")
            visible: showMoreOptions

            onClicked: moreMenu.open()

            Menu {
                id: moreMenu
                modal: true
                width: focusMenuItem.width

                onAboutToShow: root.enablePopupShield(true)
                onAboutToHide: root.enablePopupShield(false)

                CloseMenuItem {
                    text: qsTr("<b>Options</b>")
                    Accessible.name: qsTr("close options menu")
                }

                AccessibleMenuItem {
                    text: qsTr("Add user view")
                    onTriggered: addUserView()
                    MenuItemSvg { svg: SvgOutline.user }
                }

                AccessibleMenuItem {
                    text: qsTr("Add hashtag view")
                    onTriggered: addHashtagView()
                    MenuItemSvg { svg: SvgOutline.hashtag }
                }

                AccessibleMenuItem {
                    id: focusMenuItem
                    width: 250
                    text: qsTr("Add focus hashtag view")
                    onTriggered: addFocusHashtagView()
                    MenuItemSvg { svg: SvgOutline.hashtag; color: guiSettings.favoriteColor }
                }

                AccessibleMenuItem {
                    text: qsTr("Add media view")
                    onTriggered: addMediaView()
                    MenuItemSvg { svg: SvgOutline.image }
                }

                AccessibleMenuItem {
                    text: qsTr("Add video view")
                    onTriggered: addVideoView()
                    MenuItemSvg { svg: SvgOutline.film }
                }
            }
        }
        FeedAvatar {
            Layout.leftMargin: 10
            Layout.rightMargin: 10
            Layout.preferredHeight: parent.height - 10
            Layout.preferredWidth: height
            avatarUrl: header.feedAvatar
            contentMode: header.contentMode
            badgeOutlineColor: guiSettings.headerColor
            unknownSvg: defaultSvg
            visible: showAsHome && !isHomeFeed

            onClicked: header.feedAvatarClicked()

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
            font.pointSize: guiSettings.scaledFont(10/8)
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

            MouseArea {
                anchors.fill: parent
                onClicked: parent.expandFeeds()
            }

            function expandFeeds() {
                if (expandFeedsButton.visible)
                    expandFeedsButton.clicked() // qmllint disable missing-property
            }
        }

        SvgButton {
            svg: SvgOutline.language
            iconColor: guiSettings.headerTextColor
            Material.background: "transparent"
            accessibleName: qsTr("language filter active")
            visible: showLanguageFilter
            onClicked: showLanguageFilterDetails()
        }

        SvgButton {
            svg: guiSettings.getContentModeSvg(contentMode)
            iconColor: guiSettings.headerTextColor
            Material.background: "transparent"
            accessibleName: qsTr("view mode")
            visible: showViewOptions

            onClicked: viewMenu.open()

            Menu {
                id: viewMenu
                modal: true

                onAboutToShow: root.enablePopupShield(true)
                onAboutToHide: root.enablePopupShield(false)

                CloseMenuItem {
                    text: qsTr("<b>View</b>")
                    Accessible.name: qsTr("close view menu")
                }

                AccessibleMenuItem {
                    text: qsTr("Post view")
                    visible: underlyingContentMode === QEnums.CONTENT_MODE_UNSPECIFIED
                    onTriggered: {
                        contentMode = QEnums.CONTENT_MODE_UNSPECIFIED
                        viewChanged(contentMode)
                    }
                    MenuItemSvg { svg: SvgOutline.chat }
                }
                AccessibleMenuItem {
                    text: qsTr("Media view")
                    visible: underlyingContentMode === QEnums.CONTENT_MODE_UNSPECIFIED
                    onTriggered: {
                        contentMode = QEnums.CONTENT_MODE_MEDIA
                        viewChanged(contentMode)
                    }
                    MenuItemSvg { svg: SvgOutline.image }
                }
                AccessibleMenuItem {
                    text: qsTr("Media gallery")
                    visible: underlyingContentMode === QEnums.CONTENT_MODE_UNSPECIFIED
                    onTriggered: {
                        contentMode = QEnums.CONTENT_MODE_MEDIA_TILES
                        viewChanged(contentMode)
                    }
                    MenuItemSvg { svg: SvgOutline.gallery }
                }
                AccessibleMenuItem {
                    text: qsTr("Video view")
                    onTriggered: {
                        contentMode = QEnums.CONTENT_MODE_VIDEO
                        viewChanged(contentMode)
                    }
                    MenuItemSvg { svg: SvgOutline.film }
                }
                AccessibleMenuItem {
                    text: qsTr("Video gallery")
                    onTriggered: {
                        contentMode = QEnums.CONTENT_MODE_VIDEO_TILES
                        viewChanged(contentMode)
                    }
                    MenuItemSvg { svg: SvgOutline.videoGallery }
                }
            }
        }

        FeedAvatar {
            Layout.rightMargin: 10
            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
            height: parent.height - 10
            width: height
            avatarUrl: header.feedAvatar
            contentMode: header.contentMode
            badgeOutlineColor: guiSettings.headerColor
            unknownSvg: defaultSvg
            visible: !showAsHome && !isHomeFeed

            onClicked: header.feedAvatarClicked()

            Accessible.role: Accessible.Button
            Accessible.name: header.feedName
            Accessible.description: Accessible.name
            Accessible.onPressAction: header.feedAvatarClicked()
        }
        Item {
            Layout.rightMargin: 10
            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
            height: parent.height - 10
            width: height
            visible: showAsHome
            Accessible.role: Accessible.Pane

            Avatar {
                id: avatar
                width: parent.width
                height: parent.height
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

        guiSettings.notice(root, msg)
    }
}
