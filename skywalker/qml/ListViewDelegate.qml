import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Rectangle {
    required property listview list
    required property profile listCreator
    required property string listBlockedUri
    required property bool listMuted
    required property bool listSaved
    required property bool listPinned
    required property bool listHideFromTimeline
    required property bool listHideReplies
    required property bool listHideFollowing
    required property bool listSync
    property bool showList: listVisible()
    property bool allowEdit: true
    property int margin: 10
    property int maxTextLines: 1000

    id: view
    height: grid.height
    color: guiSettings.backgroundColor

    signal updateList(listview list)
    signal deleteList(listview list)
    signal blockList(listview list)
    signal unblockList(listview list, string blockedUri)
    signal muteList(listview list)
    signal unmuteList(listview list)
    signal hideList(listview list)
    signal unhideList(listview list)
    signal hideReplies(listview list, bool hide)
    signal hideFollowing(listview list, bool hide)
    signal syncList(listview list, bool sync)

    onListPinnedChanged: {
        if (!listPinned) {
            if (listSync)
                syncList(list, false)

            if (listHideReplies)
                hideReplies(list, false)

            if (listHideFollowing)
                hideFollowing(list, false)
        }
    }

    GridLayout {
        id: grid
        columns: 3
        width: parent.width
        rowSpacing: 0

        Rectangle {
            Layout.columnSpan: 3
            Layout.fillWidth: true
            Layout.preferredHeight: 10
            color: "transparent"
        }

        ListAvatar {
            Layout.leftMargin: view.margin + 8
            Layout.rightMargin: view.margin
            Layout.topMargin: 5
            Layout.preferredWidth: guiSettings.threadColumnWidth
            Layout.preferredHeight: guiSettings.threadColumnWidth
            Layout.alignment: Qt.AlignTop
            avatarUrl: showList ? list.avatarThumb : ""

            onClicked: listClicked(list)

            Accessible.role: Accessible.Button
            Accessible.name: qsTr(`show members of list ${list.name}`)
            Accessible.onPressAction: clicked()

            FavoriteStar {
                width: 20
                visible: listPinned
            }
        }

        Column {
            spacing: 0
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.rightMargin: view.margin

            SkyCleanedTextLine {
                width: parent.width
                elide: Text.ElideRight
                font.bold: true
                color: guiSettings.textColor
                plainText: list.name
            }

            AccessibleText {
                width: parent.width
                bottomPadding: 5
                elide: Text.ElideRight
                font.pointSize: guiSettings.scaledFont(7/8)
                color: guiSettings.handleColor
                text: guiSettings.listTypeName(list.purpose)
            }

            AuthorNameAndStatus {
                width: parent.width
                author: listCreator

                Accessible.role: Accessible.Link
                Accessible.name: listCreator.name
                Accessible.onPressAction: skywalker.getDetailedProfile(listCreator.did)

                MouseArea {
                    anchors.fill:  parent
                    onClicked: skywalker.getDetailedProfile(listCreator.did)
                }
            }

            Text {
                width: parent.width
                elide: Text.ElideRight
                font.pointSize: guiSettings.scaledFont(7/8)
                color: guiSettings.handleColor
                text: "@" + listCreator.handle

                Accessible.role: Accessible.Link
                Accessible.name: text
                Accessible.onPressAction: skywalker.getDetailedProfile(listCreator.did)

                MouseArea {
                    anchors.fill:  parent
                    onClicked: skywalker.getDetailedProfile(listCreator.did)
                }
            }

            ListViewerState {
                topPadding: 5
                muted: listMuted
                blockedUri: listBlockedUri
                hideFromTimeline: listHideFromTimeline
                hideReplies: listHideReplies
                hideFollowing: listHideFollowing
                sync: listSync
            }
        }

        Rectangle {
            Layout.preferredWidth: 80
            Layout.fillHeight: true
            color: "transparent"

            SvgButton {
                id: membersButton
                anchors.right: moreButton.left
                width: 40
                height: width
                svg: SvgOutline.group
                accessibleName: qsTr(`show members of list ${list.name}`)
                onClicked: root.viewListByUri(list.uri, false)
            }

            SvgButton {
                id: moreButton
                anchors.right: parent.right
                width: 40
                height: width
                svg: SvgOutline.moreVert
                accessibleName: qsTr("more options")

                onClicked: moreMenu.popup(moreButton)
            }
        }

        Rectangle {
            Layout.columnSpan: 3
            Layout.fillWidth: true
            Layout.preferredHeight: contentLabels.height + 3
            color: "transparent"

            ContentLabels {
                id: contentLabels
                anchors.left: parent.left
                anchors.leftMargin: view.margin
                anchors.right: undefined
                contentLabels: list.labels
                contentAuthorDid: list.creator.did
            }
        }

        AccessibleText {
            topPadding: 5
            Layout.columnSpan: 3
            Layout.fillWidth: true
            Layout.leftMargin: view.margin
            Layout.rightMargin: view.margin
            wrapMode: Text.Wrap
            maximumLineCount: maxTextLines
            elide: Text.ElideRight
            textFormat: Text.RichText
            color: guiSettings.textColor
            text: list.formattedDescription
            visible: text && showList

            LinkCatcher {
                containingText: list.description
            }
        }

        Rectangle {
            Layout.columnSpan: 3
            Layout.fillWidth: true
            Layout.preferredHeight: 10
            color: "transparent"
        }

        Rectangle {
            Layout.columnSpan: 3
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: guiSettings.separatorColor
        }
    }

    MouseArea {
        z: -2 // Let other mouse areas on top
        anchors.fill: parent
        onClicked: {
            console.debug("LIST CLICKED:", list.name)
            view.listClicked(list)
        }
    }

    SkyMenu {
        id: moreMenu
        width: hideListMenuItem.width

        CloseMenuItem {
            text: qsTr("<b>List</b>")
            Accessible.name: qsTr("close more options menu")
        }
        AccessibleMenuItem {
            text: qsTr("Edit")
            visible: allowEdit && isOwnList()
            onTriggered: updateList(list)

            MenuItemSvg { svg: SvgOutline.edit }
        }

        AccessibleMenuItem {
            text: qsTr("Delete")
            visible: isOwnList()
            onTriggered: deleteList(list)

            MenuItemSvg { svg: SvgOutline.delete }
        }

        AccessibleMenuItem {
            text: listMuted ? qsTr("Unmute") : qsTr("Mute")
            visible: list.purpose === QEnums.LIST_PURPOSE_MOD
            onTriggered: listMuted ? unmuteList(list) : muteList(list)
            enabled: !listBlockedUri || listMuted

            MenuItemSvg { svg: listMuted ? SvgOutline.unmute : SvgOutline.mute }
        }

        AccessibleMenuItem {
            text: listBlockedUri ? qsTr("Unblock") : qsTr("Block")
            visible: list.purpose === QEnums.LIST_PURPOSE_MOD
            onTriggered: listBlockedUri ? unblockList(list, listBlockedUri) : blockList(list)
            enabled: !listMuted || listBlockedUri

            MenuItemSvg { svg: listBlockedUri ? SvgOutline.unblock : SvgOutline.block }
        }

        AccessibleMenuItem {
            text: listSaved ? qsTr("Unsave list") : qsTr("Save list")
            enabled: listCreator.did !== skywalker.getUserDid()
            visible: list.purpose === QEnums.LIST_PURPOSE_CURATE && !isOwnList()
            onTriggered: {
                if (listSaved)
                    skywalker.favoriteFeeds.removeList(list)
                else
                    skywalker.favoriteFeeds.addList(list)

                skywalker.saveFavoriteFeeds()
            }

            MenuItemSvg { svg: listSaved ? SvgOutline.remove : SvgOutline.add }
        }

        AccessibleMenuItem {
            text: listPinned ? qsTr("Remove favorite") : qsTr("Add favorite")
            visible: list.purpose === QEnums.LIST_PURPOSE_CURATE
            onTriggered: {
                if (isOwnList()) {
                    if (listPinned)
                        skywalker.favoriteFeeds.removeList(list) // We never show own lists as saved
                    else
                        skywalker.favoriteFeeds.pinList(list, true)
                }
                else {
                    skywalker.favoriteFeeds.pinList(list, !listPinned)
                }

                skywalker.saveFavoriteFeeds()
            }

            MenuItemSvg {
                svg: listPinned ? SvgFilled.star : SvgOutline.star
                color: listPinned ? guiSettings.favoriteColor : guiSettings.textColor
            }
        }

        AccessibleMenuItem {
            id: hideListMenuItem
            width: 250
            text: listHideFromTimeline ? qsTr("Unhide list from timeline") : qsTr("Hide list from timeline")
            visible: list.purpose === QEnums.LIST_PURPOSE_CURATE && isOwnList()
            onTriggered: {
                if (listHideFromTimeline)
                    unhideList(list)
                else
                    hideList(list)
            }

            MenuItemSvg {
                svg: listHideFromTimeline ? SvgOutline.unmute : SvgOutline.mute
            }
        }

        AccessibleMenuItem {
            text: qsTr("Translate")
            visible: list.description
            onTriggered: root.translateText(list.description)

            MenuItemSvg { svg: SvgOutline.googleTranslate }
        }

        AccessibleMenuItem {
            text: qsTr("Share")
            onTriggered: skywalker.shareList(list)

            MenuItemSvg { svg: SvgOutline.share }
        }

        AccessibleMenuItem {
            text: qsTr("Report list")
            onTriggered: root.reportList(list)

            MenuItemSvg { svg: SvgOutline.report }
        }

        AccessibleMenuItem {
            text: qsTr("Emoji names")
            visible: UnicodeFonts.hasEmoji(list.description)
            onTriggered: root.showEmojiNamesList(list.description)

            MenuItemSvg { svg: SvgOutline.emojiLanguage }
        }

        AccessibleMenuItem {
            text: qsTr("Show replies")
            checkable: true
            checked: !listHideReplies
            visible: list.purpose === QEnums.LIST_PURPOSE_CURATE
            onToggled: hideReplies(list, !checked)

            MouseArea {
                anchors.fill: parent
                enabled: !listPinned
                onClicked: skywalker.showStatusMessage(qsTr("Show replies can only be disabled for favorite lists."), QEnums.STATUS_LEVEL_INFO, 10)
            }
        }

        AccessibleMenuItem {
            text: qsTr("Show following")
            checkable: true
            checked: !listHideFollowing
            visible: list.purpose === QEnums.LIST_PURPOSE_CURATE
            onToggled: hideFollowing(list, !checked)

            MouseArea {
                anchors.fill: parent
                enabled: !listPinned
                onClicked: skywalker.showStatusMessage(qsTr("Show following can only be disabled for favorite lists."), QEnums.STATUS_LEVEL_INFO, 10)
            }
        }

        AccessibleMenuItem {
            text: qsTr("Rewind on startup")
            checkable: true
            checked: listSync
            visible: list.purpose === QEnums.LIST_PURPOSE_CURATE
            onToggled: syncList(list, checked)

            MouseArea {
                anchors.fill: parent
                enabled: !listPinned
                onClicked: skywalker.showStatusMessage(qsTr("Rewinding can only be enabled for favorite lists."), QEnums.STATUS_LEVEL_INFO, 10)
            }
        }
    }

    function isOwnList() {
        return skywalker.getUserDid() === list.creator.did
    }

    function listClicked(list) {
        root.viewListByUri(list.uri, true)
    }

    function listVisible() {
        return guiSettings.feedContentVisible(list)
    }
}
