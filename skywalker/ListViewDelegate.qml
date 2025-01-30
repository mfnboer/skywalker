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
    property bool showList: listVisible()
    property bool ownLists: true
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

    GridLayout {
        id: grid
        columns: 3
        width: parent.width
        rowSpacing: 0

        Rectangle {
            Layout.columnSpan: 3
            Layout.fillWidth: true
            height: 10
            color: "transparent"
        }

        ListAvatar {
            Layout.leftMargin: view.margin
            Layout.rightMargin: view.margin
            x: 8
            y: 5
            width: guiSettings.threadColumnWidth
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
                elide: Text.ElideRight
                font.pointSize: guiSettings.scaledFont(7/8)
                color: guiSettings.handleColor
                text: guiSettings.listTypeName(list.purpose)
            }

            SkyCleanedTextLine {
                topPadding: 5
                width: parent.width
                elide: Text.ElideRight
                color: guiSettings.textColor
                plainText: listCreator.name

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
            }
        }

        Rectangle {
            width: 80
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

                onClicked: {
                    switch (list.purpose) {
                    case QEnums.LIST_PURPOSE_CURATE:
                        if (list.creator.did === skywalker.getUserDid())
                            moreMenuOwnUserList.popup(moreButton)
                        else
                            moreMenuOtherUserList.popup(moreButton)

                        break
                    case QEnums.LIST_PURPOSE_MOD:
                        if (list.creator.did === skywalker.getUserDid())
                            moreMenuOwnModList.popup(moreButton)
                        else
                            moreMenuOtherModList.popup(moreButton)
                        break
                    }
                }
            }
        }

        Rectangle {
            Layout.columnSpan: 3
            Layout.fillWidth: true
            height: contentLabels.height + 3
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

        Text {
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

            onLinkActivated: (link) => root.openLink(link)

            Accessible.role: Accessible.StaticText
            Accessible.name: list.description
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

    Menu {
        id: moreMenuOwnUserList
        modal: true

        CloseMenuItem {
            text: qsTr("<b>List</b>")
            Accessible.name: qsTr("close more options menu")
        }
        AccessibleMenuItem {
            text: qsTr("Edit")
            enabled: allowEdit
            onTriggered: updateList(list)

            MenuItemSvg { svg: SvgOutline.edit }
        }

        AccessibleMenuItem {
            text: qsTr("Delete")
            enabled: ownLists
            onTriggered: deleteList(list)

            MenuItemSvg { svg: SvgOutline.delete }
        }

        AccessibleMenuItem {
            text: listPinned ? qsTr("Remove favorite") : qsTr("Add favorite")
            onTriggered: {
                if (listPinned)
                    skywalker.favoriteFeeds.removeList(list) // We never show own lists as saved
                else
                    skywalker.favoriteFeeds.pinList(list, true)

                skywalker.saveFavoriteFeeds()
            }

            MenuItemSvg {
                svg: listPinned ? SvgFilled.star : SvgOutline.star
                color: listPinned ? guiSettings.favoriteColor : guiSettings.textColor
            }
        }

        AccessibleMenuItem {
            text: qsTr("Translate")
            enabled: list.description
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

            MenuItemSvg {
                svg: SvgOutline.report
            }
        }
    }

    Menu {
        id: moreMenuOtherUserList
        modal: true

        CloseMenuItem {
            text: qsTr("<b>List</b>")
            Accessible.name: qsTr("close more options menu")
        }
        AccessibleMenuItem {
            text: listSaved ? qsTr("Unsave list") : qsTr("Save list")
            enabled: listCreator.did !== skywalker.getUserDid()
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
            onTriggered: {
                skywalker.favoriteFeeds.pinList(list, !listPinned)
                skywalker.saveFavoriteFeeds()
            }

            MenuItemSvg {
                svg: listPinned ? SvgFilled.star : SvgOutline.star
                color: listPinned ? guiSettings.favoriteColor : guiSettings.textColor
            }
        }

        AccessibleMenuItem {
            text: qsTr("Translate")
            enabled: list.description
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

            MenuItemSvg {
                svg: SvgOutline.report
            }
        }
    }

    Menu {
        id: moreMenuOwnModList
        modal: true

        CloseMenuItem {
            text: qsTr("<b>List</b>")
            Accessible.name: qsTr("close more options menu")
        }
        AccessibleMenuItem {
            text: qsTr("Edit")
            enabled: allowEdit
            onTriggered: updateList(list)

            MenuItemSvg { svg: SvgOutline.edit }
        }

        AccessibleMenuItem {
            text: qsTr("Delete")
            enabled: ownLists
            onTriggered: deleteList(list)

            MenuItemSvg { svg: SvgOutline.delete }
        }

        AccessibleMenuItem {
            text: listMuted ? qsTr("Unmute") : qsTr("Mute")
            onTriggered: listMuted ? unmuteList(list) : muteList(list)
            enabled: !listBlockedUri || listMuted

            MenuItemSvg { svg: listMuted ? SvgOutline.unmute : SvgOutline.mute }
        }

        AccessibleMenuItem {
            text: listBlockedUri ? qsTr("Unblock") : qsTr("Block")
            onTriggered: listBlockedUri ? unblockList(list, listBlockedUri) : blockList(list)
            enabled: !listMuted || listBlockedUri

            MenuItemSvg { svg: listBlockedUri ? SvgOutline.unblock : SvgOutline.block }
        }

        AccessibleMenuItem {
            text: qsTr("Translate")
            enabled: list.description
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

            MenuItemSvg {
                svg: SvgOutline.report
            }
        }
    }

    Menu {
        id: moreMenuOtherModList
        modal: true

        CloseMenuItem {
            text: qsTr("<b>List</b>")
            Accessible.name: qsTr("close more options menu")
        }
        AccessibleMenuItem {
            text: listMuted ? qsTr("Unmute") : qsTr("Mute")
            onTriggered: listMuted ? unmuteList(list) : muteList(list)
            enabled: !listBlockedUri || listMuted

            MenuItemSvg { svg: listMuted ? SvgOutline.unmute : SvgOutline.mute }
        }

        AccessibleMenuItem {
            text: listBlockedUri ? qsTr("Unblock") : qsTr("Block")
            onTriggered: listBlockedUri ? unblockList(list, listBlockedUri) : blockList(list)
            enabled: !listMuted || listBlockedUri

            MenuItemSvg { svg: listBlockedUri ? SvgOutline.unblock : SvgOutline.block }
        }

        AccessibleMenuItem {
            text: qsTr("Translate")
            enabled: list.description
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

            MenuItemSvg {
                svg: SvgOutline.report
            }
        }
    }


    function listClicked(list) {
        root.viewListByUri(list.uri, true)
    }

    function listVisible() {
        return guiSettings.feedContentVisible(list)
    }
}
