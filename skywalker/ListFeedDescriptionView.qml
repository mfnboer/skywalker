import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyPage {
    required property var skywalker
    required property listview list
    property string listBlockedUri: list.viewer.blocked
    property bool listMuted: list.viewer.muted
    property bool isSavedList: skywalker.favoriteFeeds.isSavedFeed(list.uri)
    property bool isPinnedList: skywalker.favoriteFeeds.isPinnedFeed(list.uri)
    property bool listHideFromTimeline: skywalker.getTimelineHide().hasList(list.uri)
    property bool listSync: skywalker.getUserSettings().mustSyncFeed(skywalker.getUserDid(), list.uri)
    property bool listHideReplies: skywalker.getUserSettings().getFeedHideReplies(skywalker.getUserDid(), list.uri)
    property bool listHideFollowing: skywalker.getUserSettings().getFeedHideFollowing(skywalker.getUserDid(), list.uri)
    property int contentVisibility: QEnums.CONTENT_VISIBILITY_HIDE_POST // QEnums::ContentVisibility
    property string contentWarning: ""

    signal closed
    signal listUpdated(listview list)

    id: page

    Accessible.role: Accessible.Pane

    onIsPinnedListChanged: {
        if (!isPinnedList) {
            if (listSync)
                syncList(false)

            if (listHideReplies)
                hideReplies(false)

            if (listHideFollowing)
                hideFollowing(false)
        }
    }

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
            Layout.preferredHeight: 10
            color: "transparent"
        }

        Rectangle {
            Layout.leftMargin: 8
            Layout.topMargin: 5
            Layout.preferredWidth: 100
            Layout.preferredHeight: 100
            Layout.alignment: Qt.AlignTop

            ListAvatar {
                id: listAvatar
                width: parent.width
                height: parent.height
                avatarUrl: !contentVisible() ? "" : list.avatar
                onClicked: {
                    if (list.avatar) {
                        fullImageLoader.show(0)
                        listAvatar.visible = false
                    }
                }

                Accessible.role: Accessible.StaticText
                Accessible.name: qsTr("list avatar") + (isPinnedList ? qsTr(", one of your favorites") : "")

                FavoriteStar {
                    width: 30
                    visible: isPinnedList
                }
            }
        }

        Column {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0
            leftPadding: 10
            rightPadding: 10

            SkyCleanedText {
                width: parent.width
                elide: Text.ElideRight
                wrapMode: Text.Wrap
                maximumLineCount: 2
                font.bold: true
                font.pointSize: guiSettings.scaledFont(12/8)
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
                plainText: list.creator.name

                Accessible.role: Accessible.Link
                Accessible.name: list.creator.name
                Accessible.onPressAction: skywalker.getDetailedProfile(list.creator.did)

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

                Accessible.role: Accessible.Link
                Accessible.name: text
                Accessible.onPressAction: skywalker.getDetailedProfile(list.creator.did)

                MouseArea {
                    anchors.fill:  parent
                    onClicked: skywalker.getDetailedProfile(list.creator.did)
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

            ContentLabels {
                id: contentLabels
                anchors.left: parent.left
                anchors.right: undefined
                contentLabels: list.labels
                contentAuthorDid: list.creator.did
            }
        }

        Column {
            SvgButton {
                id: moreButton
                svg: SvgOutline.moreVert
                accessibleName: qsTr("more options")

                onClicked: moreMenu.popup(moreButton)
            }

            SvgButton {
                id: addUser
                svg: SvgOutline.addUser
                accessibleName: qsTr("add user to list")
                visible: isOwnList()
                onClicked: page.addUser()
            }
        }

        AccessibleText {
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

            LinkCatcher {
                containingText: list.description
            }
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
        id: moreMenu
        modal: true
        width: hideListMenuItem.width

        CloseMenuItem {
            text: qsTr("<b>List</b>")
            Accessible.name: qsTr("close more options menu")
        }

        AccessibleMenuItem {
            text: qsTr("Edit")
            visible: isOwnList()
            onTriggered: editList()

            MenuItemSvg { svg: SvgOutline.edit }
        }

        AccessibleMenuItem {
            text: listMuted ? qsTr("Unmute") : qsTr("Mute")
            onTriggered: listMuted ? graphUtils.unmuteList(list.uri) : graphUtils.muteList(list.uri)
            visible: list.purpose === QEnums.LIST_PURPOSE_MOD
            enabled: !listBlockedUri || listMuted

            MenuItemSvg { svg: listMuted ? SvgOutline.unmute : SvgOutline.mute }
        }

        AccessibleMenuItem {
            text: listBlockedUri ? qsTr("Unblock") : qsTr("Block")
            onTriggered: listBlockedUri ? graphUtils.unblockList(list.uri, listBlockedUri) : graphUtils.blockList(list.uri)
            visible: list.purpose === QEnums.LIST_PURPOSE_MOD
            enabled: !listMuted || listBlockedUri

            MenuItemSvg { svg: listBlockedUri ? SvgOutline.unblock : SvgOutline.block }
        }

        AccessibleMenuItem {
            text: isSavedList ? qsTr("Unsave list") : qsTr("Save list")
            visible: list.purpose === QEnums.LIST_PURPOSE_CURATE && !isOwnList()
            onTriggered: {
                if (isSavedList)
                    skywalker.favoriteFeeds.removeList(list)
                else
                    skywalker.favoriteFeeds.addList(list)

                isSavedList = !isSavedList
                isPinnedList = skywalker.favoriteFeeds.isPinnedFeed(list.uri)
                skywalker.saveFavoriteFeeds()
            }

            MenuItemSvg { svg: isSavedList ? SvgOutline.remove : SvgOutline.add }
        }

        AccessibleMenuItem {
            text: isPinnedList ? qsTr("Remove favorite") : qsTr("Add favorite")
            visible: list.purpose === QEnums.LIST_PURPOSE_CURATE
            onTriggered: {
                if (isOwnList()) {
                    if (isPinnedList)
                        skywalker.favoriteFeeds.removeList(list) // We never show own lists as saved
                    else
                        skywalker.favoriteFeeds.pinList(list, true)
                }
                else {
                    skywalker.favoriteFeeds.pinList(list, !isPinnedList)
                }

                isPinnedList = !isPinnedList
                isSavedList = skywalker.favoriteFeeds.isSavedFeed(list.uri)
                skywalker.saveFavoriteFeeds()
            }

            MenuItemSvg {
                svg: isPinnedList ? SvgFilled.star : SvgOutline.star
                color: isPinnedList ? guiSettings.favoriteColor : guiSettings.textColor
            }
        }

        AccessibleMenuItem {
            id: hideListMenuItem
            width: 250
            visible: list.purpose === QEnums.LIST_PURPOSE_CURATE && isOwnList()
            text: listHideFromTimeline ? qsTr("Unhide list from timeline") : qsTr("Hide list from timeline")
            onTriggered: {
                if (listHideFromTimeline) {
                    graphUtils.unhideList(list.uri)
                    listHideFromTimeline = false
                }
                else {
                    graphUtils.hideList(list.uri)
                }

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

            MenuItemSvg { svg: SvgOutline.smiley }
        }
        AccessibleMenuItem {
            text: qsTr("Show replies")
            checkable: true
            checked: !listHideReplies
            visible: list.purpose === QEnums.LIST_PURPOSE_CURATE
            onToggled: {
                graphUtils.hideReplies(list.uri, !checked)
                listHideReplies = !checked
            }

            MouseArea {
                anchors.fill: parent
                enabled: !isPinnedList
                onClicked: skywalker.showStatusMessage(qsTr("Show replies can only be disabled for favorite lists."), QEnums.STATUS_LEVEL_INFO, 10)
            }
        }
        AccessibleMenuItem {
            text: qsTr("Show following")
            checkable: true
            checked: !listHideFollowing
            visible: list.purpose === QEnums.LIST_PURPOSE_CURATE
            onToggled: {
                graphUtils.hideFollowing(list.uri, !checked)
                listHideFollowing = !checked
            }

            MouseArea {
                anchors.fill: parent
                enabled: !isPinnedList
                onClicked: skywalker.showStatusMessage(qsTr("Show following can only be disabled for favorite lists."), QEnums.STATUS_LEVEL_INFO, 10)
            }
        }
        AccessibleMenuItem {
            text: qsTr("Rewind on startup")
            checkable: true
            checked: listSync
            visible: list.purpose === QEnums.LIST_PURPOSE_CURATE
            onToggled: {
                graphUtils.syncList(list.uri, checked)
                listSync = checked
            }

            MouseArea {
                anchors.fill: parent
                enabled: !isPinnedList
                onClicked: skywalker.showStatusMessage(qsTr("Rewinding can only be enabled for favorite lists."), QEnums.STATUS_LEVEL_INFO, 10)
            }
        }
    }

    FullImageViewLoader {
        id: fullImageLoader
        thumbImageViewList: [listAvatar.getImage()]
        images: [list.imageView]
        onFinished: listAvatar.visible = true
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
        onHideListOk: {
            listHideFromTimeline = true
            statusPopup.show(qsTr("List hidden from timeline."), QEnums.STATUS_LEVEL_INFO, 2)
        }
        onHideListFailed: (error) => statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)

    }

    ProfileUtils {
        id: profileUtils
        skywalker: page.skywalker

        onProfileViewOk: (profile, listItemUri) => authorListView.model.prependAuthor(profile, listItemUri)
        onProfileViewFailed: (error) => statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
    }


    function editList() {
        let component = guiSettings.createComponent("EditList.qml")
        let editPage = component.createObject(page, {
                skywalker: skywalker,
                purpose: list.purpose,
                list: list
            })
        editPage.onListUpdated.connect((cid, name, description, embeddedLinks, avatar) => {
            list = graphUtils.makeListView(list.uri, cid, name, list.purpose, avatar,
                                           skywalker.getUserProfile(), description,
                                           embeddedLinks)
            page.listUpdated(list)
            root.popStack()
        })
        editPage.onClosed.connect(() => { root.popStack() })
        root.pushStack(editPage)
    }

    function addUser() {
        let component = guiSettings.createComponent("SearchAuthor.qml")
        let searchPage = component.createObject(page, { skywalker: skywalker })
        searchPage.onAuthorClicked.connect((profile) => { // qmllint disable missing-property
            graphUtils.addListUser(list.uri, profile)
            root.popStack()
        })
        searchPage.onClosed.connect(() => { root.popStack() })
        root.pushStack(searchPage)
    }

    function syncList(sync) {
        graphUtils.syncList(list.uri, sync)
        listSync = sync
    }

    function hideReplies(hide) {
        graphUtils.hideReplies(list.uri, hide)
        listHideReplies = hide
    }

    function hideFollowing(hide) {
        graphUtils.hideFollowing(list.uri, hide)
        listHideFollowing = hide
    }

    function isOwnList() {
        return skywalker.getUserDid() === list.creator.did
    }

    function contentVisible() {
        if (list.viewer.blockedBy)
            return false

        return contentVisibility === QEnums.CONTENT_VISIBILITY_SHOW || showWarnedMedia
    }

    Component.onCompleted: {
        contentVisibility = skywalker.getContentVisibility(list.labels)
        contentWarning = skywalker.getContentWarning(list.labels)
    }
}
