import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

Rectangle {
    required property var skywalker
    required property string feedName
    property SvgImage defaultSvg: SvgFilled.feed
    property string feedAvatar
    property bool showAsHome: false
    property bool isHomeFeed: false
    property bool showLanguageFilter: false
    property list<language> filteredLanguages
    property bool showPostWithMissingLanguage: true
    property bool showMoreOptions: false

    signal closed
    signal feedAvatarClicked
    signal addUserView
    signal addFocusHashtagView

    id: header
    width: parent.width
    height: guiSettings.headerHeight
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
            svg: SvgOutline.moreVert
            accessibleName: qsTr("more options")
            visible: showMoreOptions

            onClicked: moreMenu.open()

            Menu {
                id: moreMenu
                modal: true
                width: focusMenuItem.width // TODO

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
                    id: focusMenuItem
                    width: 250
                    text: qsTr("Add focus hashtag view")
                    onTriggered: addFocusHashtagView()
                    MenuItemSvg { svg: SvgOutline.hashtag }
                }
            }
        }
        FeedAvatar {
            Layout.leftMargin: 10
            Layout.rightMargin: 10
            Layout.preferredHeight: parent.height - 10
            Layout.preferredWidth: height
            avatarUrl: header.feedAvatar
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
                    expandFeedsButton.onClicked() // qmllint disable missing-proprety
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

        FeedAvatar {
            Layout.rightMargin: 10
            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
            height: parent.height - 10
            width: height
            avatarUrl: header.feedAvatar
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
