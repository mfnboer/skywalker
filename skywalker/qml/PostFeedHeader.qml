import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

Rectangle {
    property string userDid
    property Skywalker skywalker: root.getSkywalker(userDid)
    property UserSettings userSettings: skywalker.getUserSettings()
    property bool reverseFeed: false
    required property string feedName
    property string feedKey
    property string subTitle
    property SvgImage defaultSvg: SvgFilled.feed
    property string feedAvatar
    property int contentMode: QEnums.CONTENT_MODE_UNSPECIFIED
    property int underlyingContentMode: QEnums.CONTENT_MODE_UNSPECIFIED
    property bool showAsHome: false
    property bool isHomeFeed: false // TODO: isFollowingFeed
    property bool showLanguageFilter: false
    property list<language> filteredLanguages
    property bool showPostWithMissingLanguage: true
    property bool showMoreOptions: false
    property bool showViewOptions: false
    property bool showFavoritesPlaceHolder: false
    property int bottomMargin: 0
    readonly property int favoritesY: headerRow.height

    signal closed
    signal feedAvatarClicked(point clickPoint)
    signal feedAvatarPressAndHold
    signal addUserView
    signal addHashtagView
    signal addFocusHashtagView
    signal addVideoView
    signal addMediaView
    signal filterStatistics
    signal viewChanged(int contentMode)
    signal newReverseFeed(bool reverse)

    id: header
    width: parent.width
    height: headerRow.y + headerRow.height + (showFavoritesPlaceHolder ? guiSettings.tabBarHeight : 0) + bottomMargin
    z: guiSettings.headerZLevel
    color: guiSettings.headerColor

    Accessible.role: Accessible.Pane

    RowLayout {
        id: headerRow
        width: parent.width
        height: header.visible ? guiSettings.headerHeight : 0
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
            Layout.leftMargin: showMoreOptions ? 10 : 0
            active: showMoreOptions

            sourceComponent: FeedAvatar {
                id: moreButton
                height: guiSettings.headerHeight - 10
                width: height
                userDid: header.userDid
                badgeOutlineColor: guiSettings.headerColor
                unknownSvg: SvgFilled.following

                onClicked: moreMenu.open()

                TimelineOptionsMenu {
                    id: moreMenu
                    reverseFeed: header.reverseFeed

                    onAddUserView: header.addUserView()
                    onAddHashtagView: header.addHashtagView()
                    onAddFocusHashtagView: header.addFocusHashtagView()
                    onAddMediaView: header.addMediaView()
                    onAddVideoView: header.addVideoView()
                    onFilterStatistics: header.filterStatistics()
                    onNewReverseFeed: (reverse) => header.newReverseFeed(reverse)
                }

                Accessible.role: Accessible.Button
                Accessible.name: qsTr("timeline options")
                Accessible.onPressAction: clicked(Qt.point(0, 0))
            }
        }

        FeedAvatar {
            id: avatarLeftSide
            Layout.leftMargin: 10
            Layout.rightMargin: 10
            Layout.preferredHeight: parent.height - 10
            Layout.preferredWidth: height
            userDid: header.userDid
            avatarUrl: header.feedAvatar
            contentMode: header.contentMode
            badgeOutlineColor: guiSettings.headerColor
            unknownSvg: defaultSvg
            visible: showAsHome && !header.isHomeFeed

            onClicked: (clickPoint) => {
                const mousePoint = clickPoint ?
                    mapToItem(header, clickPoint) :
                    mapToItem(header, 0, 0)

                header.feedAvatarClicked(mousePoint)
            }
            onPressAndHold: header.feedAvatarPressAndHold()

            Accessible.role: Accessible.Button
            Accessible.name: header.feedName
            Accessible.onPressAction: clicked(Qt.point(0, 0))
        }

        Column {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter

            AccessibleText {
                id: headerTexts
                width: parent.width
                leftPadding: avatarLeftSide.visible ? 0 : 10
                rightPadding: showAsHome ? expandFeedsButton.width : 0
                elide: Text.ElideRight
                font.bold: true
                font.pointSize: subTitle ? guiSettings.scaledFont(1) : guiSettings.scaledFont(10/8)
                color: guiSettings.headerTextColor
                text: (skywalker.favoriteFeeds.homeFeedUri === header.feedKey ? "🏠 " : "") + header.feedName

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

            AccessibleText {
                width: parent.width
                leftPadding: avatarLeftSide.visible ? 0 : 10
                color: guiSettings.handleColor
                font.pointSize: guiSettings.scaledFont(7/8)
                elide: Text.ElideRight
                text: subTitle
                visible: subTitle
            }
        }

        LanguageFilterButton {
            filteredLanguages: header.filteredLanguages
            showPostWithMissingLanguage: header.showPostWithMissingLanguage
            visible: showLanguageFilter
        }

        SvgPlainButton {
            svg: guiSettings.getContentModeSvg(contentMode)
            iconColor: guiSettings.headerTextColor
            accessibleName: qsTr("view mode")
            visible: showViewOptions

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
            visible: !showAsHome && !header.isHomeFeed

            onClicked: (clickPoint) => {
                const mousePoint = clickPoint ?
                    mapToItem(header, clickPoint) :
                    mapToItem(header, 0, 0)

                header.feedAvatarClicked(mousePoint)
            }

            Accessible.role: Accessible.Button
            Accessible.name: header.feedName
            Accessible.description: Accessible.name
            Accessible.onPressAction: header.feedAvatarClicked(Qt.point(0, 0))
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
            visible: showAsHome
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

    Component {
        id: viewMenuComponent

        ContentViewMenu {
            id: viewMenu
            contentMode: header.contentMode
            underlyingContentMode: header.underlyingContentMode

            onViewChanged: (contentMode) => {
                header.viewChanged(contentMode)
            }
        }
    }
}
