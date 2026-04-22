import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Rectangle {
    property string userDid
    property Skywalker skywalker: root.getSkywalker(userDid)
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
            userDid: view.userDid
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
                userDid: view.userDid
                author: listCreator

                Accessible.role: Accessible.Link
                Accessible.name: listCreator.name
                Accessible.onPressAction: skywalker.getDetailedProfile(listCreator.did)

                SkyMouseArea {
                    anchors.fill:  parent
                    onClicked: skywalker.getDetailedProfile(listCreator.did)
                }
            }

            AccessibleText {
                width: parent.width
                elide: Text.ElideRight
                font.pointSize: guiSettings.scaledFont(7/8)
                color: guiSettings.handleColor
                text: "@" + listCreator.handle

                Accessible.role: Accessible.Link
                Accessible.name: text
                Accessible.onPressAction: skywalker.getDetailedProfile(listCreator.did)

                SkyMouseArea {
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
                onClicked: root.viewListByUri(list.uri, false, userDid)
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
                contentAuthor: list.creator
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
                author: list.creator
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

    SkyMouseArea {
        z: -2 // Let other mouse areas on top
        anchors.fill: parent
        onClicked: {
            console.debug("LIST CLICKED:", list.name)
            view.listClicked(list)
        }
    }

    SkyMenu {
        id: moreMenu
        menuWidth: 250

        SkyMenuButton {
            text: qsTr("Edit")
            svg: SvgOutline.edit
            popup: moreMenu
            visible: allowEdit && isOwnList()
            onClicked: updateList(list)
        }

        SkyMenuButton {
            text: qsTr("Delete")
            svg: SvgOutline.delete
            popup: moreMenu
            visible: isOwnList()
            onClicked: deleteList(list)
        }

        SkyMenuButton {
            text: listMuted ? qsTr("Unmute") : qsTr("Mute")
            svg: listMuted ? SvgOutline.unmute : SvgOutline.mute
            popup: moreMenu
            visible: list.purpose === QEnums.LIST_PURPOSE_MOD
            onClicked: listMuted ? unmuteList(list) : muteList(list)
            enabled: !listBlockedUri || listMuted
        }

        SkyMenuButton {
            text: listBlockedUri ? qsTr("Unblock") : qsTr("Block")
            svg: listBlockedUri ? SvgOutline.unblock : SvgOutline.block
            popup: moreMenu
            visible: list.purpose === QEnums.LIST_PURPOSE_MOD
            onClicked: listBlockedUri ? unblockList(list, listBlockedUri) : blockList(list)
            enabled: !listMuted || listBlockedUri
        }

        SkyMenuButton {
            text: listSaved ? qsTr("Unsave list") : qsTr("Save list")
            svg: listSaved ? SvgOutline.remove : SvgOutline.add
            popup: moreMenu
            enabled: listCreator.did !== skywalker.getUserDid()
            visible: list.purpose === QEnums.LIST_PURPOSE_CURATE && !isOwnList()
            onClicked: {
                if (listSaved)
                    skywalker.favoriteFeeds.removeList(list)
                else
                    skywalker.favoriteFeeds.addList(list)

                skywalker.saveFavoriteFeeds()
            }
        }

        SkyMenuButton {
            text: listPinned ? qsTr("Remove favorite") : qsTr("Add favorite")
            svg: listPinned ? SvgFilled.star : SvgOutline.star
            svgColor: listPinned ? guiSettings.favoriteColor : guiSettings.textColor
            popup: moreMenu
            visible: list.purpose === QEnums.LIST_PURPOSE_CURATE
            onClicked: {
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
        }

        SkyMenuButton {
            id: hideListMenuItem
            text: listHideFromTimeline ? qsTr("Unhide list from timeline") : qsTr("Hide list from timeline")
            svg: listHideFromTimeline ? SvgOutline.unmute : SvgOutline.mute
            popup: moreMenu
            visible: list.purpose === QEnums.LIST_PURPOSE_CURATE && isOwnList()
            onClicked: {
                if (listHideFromTimeline)
                    unhideList(list)
                else
                    hideList(list)
            }
        }

        SkyMenuButton {
            text: qsTr("Translate")
            svg: SvgOutline.googleTranslate
            popup: moreMenu
            visible: list.description
            onClicked: root.translateText(list.description)
        }

        SkyMenuButton {
            text: qsTr("Share")
            svg: SvgOutline.share
            popup: moreMenu
            onClicked: skywalker.getShareUtils().shareList(list)
        }

        SkyMenuButton {
            text: qsTr("Copy list link")
            svg: SvgOutline.link
            popup: moreMenu
            onClicked: skywalker.getShareUtils().copyUriToClipboard(list.uri)
        }

        SkyMenuButton {
            text: qsTr("Report list")
            svg: SvgOutline.report
            popup: moreMenu
            onClicked: root.reportList(list, userDid)
        }

        SkyMenuButton {
            text: qsTr("Emoji names")
            svg: SvgOutline.emojiLanguage
            popup: moreMenu
            visible: UnicodeFonts.hasEmoji(list.description)
            onClicked: root.showEmojiNamesList(list.description)
        }

        AccessibleMenuItem {
            text: qsTr("Show replies")
            checkable: true
            checked: !listHideReplies
            visible: list.purpose === QEnums.LIST_PURPOSE_CURATE
            onToggled: hideReplies(list, !checked)

            SkyMouseArea {
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

            SkyMouseArea {
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

            SkyMouseArea {
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
        root.viewListByUri(list.uri, true, userDid)
    }

    function listVisible() {
        return guiSettings.feedContentVisible(list, userDid)
    }
}
