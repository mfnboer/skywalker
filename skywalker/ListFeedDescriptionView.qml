import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Page {
    required property var skywalker
    required property listview list
    property string listBlockedUri: list.viewer.blocked
    property bool listMuted: list.viewer.muted

    property bool isSavedList: skywalker.favoriteFeeds.isSavedFeed(list.uri)
    property bool isPinnedList: skywalker.favoriteFeeds.isPinnedFeed(list.uri)

    signal closed
    signal listUpdated(listview list)

    id: page

    header: SimpleHeader {
        text: guiSettings.listTypeName(list.purpose)
        onBack: closed()
    }

    GridLayout {
        id: grid
        rowSpacing: 0
        columns: 3
        x: 10
        width: parent.width - 20

        Rectangle {
            Layout.columnSpan: 3
            Layout.fillWidth: true
            height: 10
            color: "transparent"
        }

        ListAvatar {
            x: 8
            y: 5
            width: 100
            Layout.alignment: Qt.AlignTop
            avatarUrl: list.avatar
            onClicked: {
                if (list.avatar)
                    root.viewFullImage([list.imageView], 0)
            }

            FavoriteStar {
                width: 30
                visible: isPinnedList
            }
        }

        Column {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0
            leftPadding: 10
            rightPadding: 10

            Text {
                width: parent.width
                elide: Text.ElideRight
                wrapMode: Text.Wrap
                maximumLineCount: 2
                font.bold: true
                font.pointSize: guiSettings.scaledFont(12/8)
                color: guiSettings.textColor
                text: list.name
            }

            Text {
                width: parent.width
                elide: Text.ElideRight
                font.pointSize: guiSettings.scaledFont(7/8)
                color: guiSettings.handleColor
                text: guiSettings.listTypeName(list.purpose)
            }

            Text {
                topPadding: 5
                width: parent.width
                elide: Text.ElideRight
                color: guiSettings.textColor
                text: list.creator.name

                MouseArea {
                    anchors.fill:  parent
                    onClicked: skywalker.getDetailedProfile(list.creator.did)
                }
            }

            Text {
                topPadding: 2
                width: parent.width
                elide: Text.ElideRight
                font.pointSize: guiSettings.scaledFont(7/8)
                color: guiSettings.handleColor
                text: "@" + list.creator.handle

                MouseArea {
                    anchors.fill:  parent
                    onClicked: skywalker.getDetailedProfile(list.creator.did)
                }
            }

            ListViewerState {
                topPadding: 5
                muted: listMuted
                blockedUri: listBlockedUri
            }
        }

        Column {
            SvgButton {
                id: moreButton
                svg: svgOutline.moreVert

                onClicked: {
                    switch (list.purpose) {
                    case QEnums.LIST_PURPOSE_CURATE:
                        if (list.creator.did === skywalker.getUserDid())
                            moreMenuOwnUserList.popup(moreButton)
                        else
                            moreMenuOtherUserList.popup(moreButton)

                        break
                    case QEnums.LIST_PURPOSE_MOD:
                        moreMenuModList.popup(moreButton)
                        break
                    }
                }
            }

            SvgButton {
                id: addUser
                svg: svgOutline.addUser
                visible: isOwnList()
                onClicked: page.addUser()
            }
        }

        Text {
            topPadding: 5
            bottomPadding: 10
            Layout.columnSpan: 3
            Layout.fillWidth: true
            wrapMode: Text.Wrap
            maximumLineCount: 1000
            elide: Text.ElideRight
            textFormat: Text.RichText
            color: guiSettings.textColor
            text: list.formattedDescription

            onLinkActivated: (link) => root.openLink(link)
        }
    }

    AuthorListView {
        id: authorListView
        width: parent.width
        anchors.top: grid.bottom
        anchors.bottom: parent.bottom
        title: ""
        skywalker: page.skywalker
        modelId: skywalker.createAuthorListModel(QEnums.AUTHOR_LIST_LIST_MEMBERS, list.uri)
        allowDeleteItem: isOwnList()
        listUri: list.uri
        clip: true

        Component.onCompleted: skywalker.getAuthorList(modelId)
    }

    Menu {
        id: moreMenuOwnUserList

        MenuItem {
            text: qsTr("Edit")
            onTriggered: editList()

            MenuItemSvg { svg: svgOutline.edit }
        }

        MenuItem {
            text: isPinnedList ? qsTr("Remove favorite") : qsTr("Add favorite")
            onTriggered: {
                if (isPinnedList)
                    skywalker.favoriteFeeds.removeList(list) // We never show own lists as saved
                else
                    skywalker.favoriteFeeds.pinList(list, true)

                isPinnedList = !isPinnedList
                isSavedList = skywalker.favoriteFeeds.isSavedFeed(list.uri)
                skywalker.saveFavoriteFeeds()
            }

            MenuItemSvg {
                svg: isPinnedList ? svgFilled.star : svgOutline.star
                color: isPinnedList ? guiSettings.favoriteColor : guiSettings.textColor
            }
        }

        MenuItem {
            text: qsTr("Translate")
            onTriggered: root.translateText(list.description)

            MenuItemSvg { svg: svgOutline.googleTranslate }
        }
        MenuItem {
            text: qsTr("Share")
            onTriggered: skywalker.shareList(list)

            MenuItemSvg { svg: svgOutline.share }
        }
        MenuItem {
            text: qsTr("Report list")
            onTriggered: root.reportList(list)

            MenuItemSvg { svg: svgOutline.report }
        }
    }

    Menu {
        id: moreMenuOtherUserList

        MenuItem {
            text: isSavedList ? qsTr("Unsave list") : qsTr("Save list")
            onTriggered: {
                if (isSavedList)
                    skywalker.favoriteFeeds.removeList(list)
                else
                    skywalker.favoriteFeeds.addList(list)

                isSavedList = !isSavedList
                isPinnedList = skywalker.favoriteFeeds.isPinnedFeed(list.uri)
                skywalker.saveFavoriteFeeds()
            }

            MenuItemSvg { svg: isSavedList ? svgOutline.remove : svgOutline.add }
        }
        MenuItem {
            text: isPinnedList ? qsTr("Remove favorite") : qsTr("Add favorite")
            onTriggered: {
                skywalker.favoriteFeeds.pinList(list, !isPinnedList)
                isPinnedList = !isPinnedList
                isSavedList = skywalker.favoriteFeeds.isSavedFeed(list.uri)
                skywalker.saveFavoriteFeeds()
            }

            MenuItemSvg {
                svg: isPinnedList ? svgFilled.star : svgOutline.star
                color: isPinnedList ? guiSettings.favoriteColor : guiSettings.textColor
            }
        }

        MenuItem {
            text: qsTr("Translate")
            onTriggered: root.translateText(list.description)

            MenuItemSvg { svg: svgOutline.googleTranslate }
        }
        MenuItem {
            text: qsTr("Share")
            onTriggered: skywalker.shareList(list)

            MenuItemSvg { svg: svgOutline.share }
        }
        MenuItem {
            text: qsTr("Report list")
            onTriggered: root.reportList(list)

            MenuItemSvg { svg: svgOutline.report }
        }
    }

    Menu {
        id: moreMenuModList

        MenuItem {
            text: qsTr("Edit")
            onTriggered: editList()
            enabled: isOwnList()

            MenuItemSvg { svg: svgOutline.edit }
        }

        MenuItem {
            text: listMuted ? qsTr("Unmute") : qsTr("Mute")
            onTriggered: listMuted ? graphUtils.unmuteList(list.uri) : graphUtils.muteList(list.uri)
            enabled: !listBlockedUri || listMuted

            MenuItemSvg { svg: listMuted ? svgOutline.unmute : svgOutline.mute }
        }

        MenuItem {
            text: listBlockedUri ? qsTr("Unblock") : qsTr("Block")
            onTriggered: listBlockedUri ? graphUtils.unblockList(list.uri, listBlockedUri) : graphUtils.blockList(list.uri)
            enabled: !listMuted || listBlockedUri

            MenuItemSvg { svg: listBlockedUri ? svgOutline.unblock : svgOutline.block }
        }

        MenuItem {
            text: qsTr("Translate")
            onTriggered: root.translateText(list.description)

            MenuItemSvg { svg: svgOutline.googleTranslate }
        }
        MenuItem {
            text: qsTr("Share")
            onTriggered: skywalker.shareList(list)

            MenuItemSvg { svg: svgOutline.share }
        }
        MenuItem {
            text: qsTr("Report list")
            onTriggered: root.reportList(list)

            MenuItemSvg { svg: svgOutline.report }
        }
    }

    GraphUtils {
        id: graphUtils
        skywalker: page.skywalker

        onAddListUserOk: (did, itemUri, itemCid) => profileUtils.getProfileView(did, itemUri)
        onAddListUserFailed: (error) => statusPopup.show(qsTr(`Failed to add user: ${error}`), QEnums.STATUS_LEVEL_ERROR)
        onBlockListOk: (uri) => {
            listBlockedUri = uri
            authorListView.refresh()
        }
        onBlockListFailed: (error) => statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
        onUnblockListOk: {
            listBlockedUri = ""
            authorListView.refresh()
        }
        onUnblockListFailed: (error) => statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
        onMuteListOk: {
            listMuted = true
            authorListView.refresh()
        }
        onMuteListFailed: (error) => statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
        onUnmuteListOk: {
            listMuted = false
            authorListView.refresh()
        }
        onUnmuteListFailed: (error) => statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
    }

    ProfileUtils {
        id: profileUtils
        skywalker: page.skywalker

        onProfileViewOk: (profile, listItemUri) => authorListView.model.prependAuthor(profile, listItemUri)
        onProfileViewFailed: (error) => statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
    }

    GuiSettings {
        id: guiSettings
    }

    function editList() {
        let component = Qt.createComponent("EditList.qml")
        let editPage = component.createObject(page, {
                skywalker: skywalker,
                purpose: list.purpose,
                list: list
            })
        editPage.onListUpdated.connect((cid, name, description, avatar) => {
            list = graphUtils.makeListView(list.uri, cid, name, list.purpose, avatar,
                                           skywalker.getUserProfile(), description)
            page.listUpdated(list)
            root.popStack()
        })
        editPage.onClosed.connect(() => { root.popStack() })
        root.pushStack(editPage)
    }

    function addUser() {
        let component = Qt.createComponent("SearchAuthor.qml")
        let searchPage = component.createObject(page, { skywalker: skywalker })
        searchPage.onAuthorClicked.connect((profile) => {
            graphUtils.addListUser(list.uri, profile.did)
            root.popStack()
        })
        searchPage.onClosed.connect(() => { root.popStack() })
        root.pushStack(searchPage)
    }

    function isOwnList() {
        return skywalker.getUserDid() === list.creator.did
    }
}
