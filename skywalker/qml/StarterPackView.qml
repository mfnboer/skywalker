import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyPage {
    property string userDid
    property Skywalker skywalker: root.getSkywalker(userDid)
    required property starterpackview starterPack
    readonly property int postFeedModelId: skywalker.createPostFeedModel(starterPack.list)
    readonly property int feedListModelId: skywalker.createFeedListModel()
    readonly property bool editMode: false
    readonly property int margin: 10
    readonly property string sideBarTitle: qsTr("Starter pack")
    readonly property SvgImage sideBarSvg: SvgOutline.starterpack

    signal closed

    id: page

    header: SimpleHeader {
        userDid: page.userDid
        text: sideBarTitle
        visible: !root.showSideBar
        onBack: page.closed()
    }

    SwipeListView {
        id: feedStack
        width: parent.width
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        currentIndex: feedsBar.currentIndex
        headerHeight: starterPackHeader.height + feedsBar.height + feedsSeparator.height
        headerScrollHeight: starterPackHeader.height

        onCurrentIndexChanged: feedsBar.setCurrentIndex(currentIndex)

        AuthorListView {
            id: authorListView
            title: ""
            userDid: page.userDid
            modelId: skywalker.createAuthorListModel(QEnums.AUTHOR_LIST_LIST_MEMBERS, starterPack.list.uri)
            listUri: starterPack.list.uri
            allowDeleteItem: true
            clip: true

            header: PlaceholderHeader { height: feedStack.headerHeight }
            headerPositioning: ListView.InlineHeader
            footer: null

            Component.onCompleted: skywalker.getAuthorList(modelId, 100)
        }

        SkyListView {
            id: feedListView
            model: skywalker.getFeedListModel(feedListModelId)
            boundsBehavior: Flickable.StopAtBounds
            clip: true

            header: PlaceholderHeader { height: feedStack.headerHeight }
            headerPositioning: ListView.InlineHeader

            delegate: GeneratorViewDelegate {
                required property int index

                width: feedListView.width
                userDid: page.userDid
                allowDelete: true

                onHideFollowing: (feed, hide) => feedUtils.hideFollowing(feed.uri, hide)
                onSyncFeed: (feed, sync) => feedUtils.syncFeed(feed.uri, sync)
                onDeleteFeed: (feed) => graphUtils.removeFeedFromStarterPack(feed, index)
            }

            EmptyListIndication {
                y: feedStack.headerHeight
                svg: SvgOutline.noPosts
                text: qsTr("No feeds")
                list: feedListView
            }
        }

        SkyListView {
            id: postListView
            model: skywalker.getPostFeedModel(postFeedModelId)
            clip: true

            header: PlaceholderHeader { height: feedStack.headerHeight }
            headerPositioning: ListView.InlineHeader

            delegate: PostFeedViewDelegate {
                width: postListView.width
            }

            SwipeView.onIsCurrentItemChanged: {
                if (!SwipeView.isCurrentItem)
                    cover()
            }

            FlickableRefresher {
                inProgress: postListView.model && postListView.model.getFeedInProgress
                topOvershootFun: () => skywalker.getListFeed(postFeedModelId)
                bottomOvershootFun: () => skywalker.getListFeedNextPage(postFeedModelId)
                topText: qsTr("Pull down to refresh feed")
            }

            EmptyListIndication {
                y: feedStack.headerHeight
                svg: SvgOutline.noPosts
                text: qsTr("Feed is empty")
                list: postListView
            }

            BusyIndicator {
                id: busyIndicator
                anchors.centerIn: parent
                running: postListView.model && postListView.model.getFeedInProgress
            }
        }

        Component.onCompleted: {
            if (starterPack.feeds.length === 0 && !editMode)
                removeItem(feedListView)
        }
    }

    Column {
        id: starterPackHeader
        x: margin
        y: Math.max(feedStack.headerTopMinY, feedStack.headerTopMaxY, -height)
        width: parent.width - 2 * margin

        GridLayout {
            width: parent.width
            columns: 3
            rowSpacing: 0

            Rectangle {
                Layout.rowSpan: 2
                Layout.preferredWidth: guiSettings.threadColumnWidth
                Layout.preferredHeight: guiSettings.threadColumnWidth
                color: "transparent"

                SkySvg {
                    width: parent.width
                    height: parent.height
                    color: guiSettings.starterpackColor
                    svg: SvgOutline.starterpack
                }
            }

            AccessibleText {
                topPadding: 10
                Layout.fillWidth: true
                elide: Text.ElideRight
                font.bold: true
                color: guiSettings.textColor
                text: starterPack.name
            }

            Item {
                Layout.preferredWidth: addUserButton.width
                Layout.preferredHeight: addUserButton.height
                Layout.rowSpan: 2
                Layout.alignment: Qt.AlignVCenter

                SvgButton {
                    id: addUserButton
                    svg: feedsBar.currentIndex === feedsBar.indexUsers ? SvgOutline.addUser : SvgOutline.feed
                    accessibleName: feedsBar.currentIndex === feedsBar.indexUsers ? qsTr("add user to starter pack") : qsTr("add feed")
                    visible: editMode && feedsBar.currentIndex <= feedsBar.indexFeeds
                    onClicked: {
                        if (feedsBar.currentIndex === feedsBar.indexUsers)
                            addUser()
                        else if (feedsBar.currentIndex === feedsBar.indexFeeds)
                            addFeed()
                    }
                }
            }

            AccessibleText {
                Layout.fillWidth: true
                Layout.fillHeight: true
                elide: Text.ElideRight
                font.pointSize: guiSettings.scaledFont(7/8)
                color: guiSettings.handleColor
                text: qsTr(`by @${starterPack.creator.handle}`)
            }
        }

        ContentLabels {
            id: contentLabels
            anchors.left: parent.left
            anchors.leftMargin: margin
            anchors.right: undefined
            userDid: page.userDid
            contentLabels: starterPack.labels
            contentAuthor: starterPack.creator
        }

        SkyCleanedText {
            topPadding: 10
            width: parent.width
            wrapMode: Text.Wrap
            elide: Text.ElideRight
            textFormat: Text.RichText
            maximumLineCount: 25
            color: guiSettings.textColor
            plainText: starterPack.formattedDescription
            visible: starterPack.description
        }
    }

    SkyTabBar {
        readonly property int indexUsers: 0
        readonly property int indexFeeds: 1

        id: feedsBar
        anchors.top: starterPackHeader.bottom
        width: parent.width

        AccessibleTabButton {
            text: qsTr("People")
        }
        AccessibleTabButton {
            id: feedsTab
            text: qsTr("Feeds")
        }
        AccessibleTabButton {
            text: qsTr("Posts")
        }

        Component.onCompleted: {
            // Just making the tab invisible does not help against swiping. You can still
            // swipe to an invisible tab.
            if (starterPack.feeds.length === 0 && !editMode)
                removeItem(feedsTab)
        }
    }

    Rectangle {
        id: feedsSeparator
        anchors.top: feedsBar.bottom
        width: parent.width
        height: 1
        color: guiSettings.separatorColor
    }

    SvgPlainButton {
        id: moreButton
        parent: page.header.visible ? page.header : page
        anchors.right: parent.right
        anchors.rightMargin: page.margin
        svg: SvgOutline.moreVert
        accessibleName: qsTr("more options")
        onClicked: moreMenu.open()

        SkyMenu {
            id: moreMenu

            TranslateMenuButton {
                popup: moreMenu
                enabled: starterPack.description
                onClicked: root.translateText(starterPack.description)
            }
            SkyMenuButton {
                text: qsTr("Share")
                svg: SvgOutline.share
                popup: moreMenu
                onClicked: skywalker.getShareUtils().shareStarterPack(starterPack)
            }
            SkyMenuButton {
                text: qsTr("Copy starter pack link")
                svg: SvgOutline.link
                popup: moreMenu
                onClicked: skywalker.getShareUtils().copyUriToClipboard(starterPack.uri)
            }
            SkyMenuButton {
                text: qsTr("Copy to list")
                svg: SvgOutline.list
                popup: moreMenu
                onClicked: copyStarterPackToList()
            }
            SkyMenuButton {
                text: qsTr("Report starter pack")
                svg: SvgOutline.report
                popup: moreMenu
                onClicked: root.reportStarterPack(starterPack, userDid)
            }
            SkyMenuButton {
                text: qsTr("Emoji names")
                svg: SvgOutline.emojiLanguage
                popup: moreMenu
                visible: UnicodeFonts.hasEmoji(starterPack.description)
                onClicked: root.showEmojiNamesList(starterPack.description)
            }
        }
    }

    // Give the network some time after creating a list before retrieving it.
    // Sometimes you get a not-found error when you retrieve it too quickly.
    Timer {
        property string listUri

        id: getListViewTimer
        interval: 500
        onTriggered: graphUtils.getListView(listUri)

        function go(uri) {
            listUri = uri
            start()
        }
    }

    FeedUtils {
        id: feedUtils
        skywalker: page.skywalker
    }

    GraphUtils {
        property var addingFeed
        property var removingFeed
        property int removingFeedIndex

        id: graphUtils
        skywalker: page.skywalker

        onCreatedListFromStarterPackOk: (pack, listUri, listCid) => {
            skywalker.showStatusMessage(qsTr("List created"), QEnums.STATUS_LEVEL_INFO)
            getListViewTimer.go(listUri)
        }

        onCreatedListFromStarterPackFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)

        onGetListOk: (did, list) => root.viewListFeedDescription(list, did)
        onGetListFailed: (error) => {
            // The network may take a while before you can retrieve a new list.
            console.warn(error)
            skywalker.showStatusMessage(qsTr("You can find the new list in your overview of user lists"), QEnums.STATUS_LEVEL_INFO)
        }

        onAddListUserOk: (did, itemUri, itemCid) => profileUtils.getProfileView(did, itemUri)
        onAddListUserFailed: (error) => skywalker.showStatusMessage(qsTr(`Failed to add user: ${error}`), QEnums.STATUS_LEVEL_ERROR)

        onAddStarterPackFeedOk: (starterPackUri, feedUri) => {
            if (feedUri === addingFeed.uri)
                feedListView.model.prependFeed(addingFeed)
            else
                console.warn("Feed uri mismatch:", feedUri, "adding:", addingFeed.uri)
        }

        onAddStarterPackFeedFailed: (error) => skywalker.showStatusMessage(qsTr(`Failed to add feed: ${error}`), QEnums.STATUS_LEVEL_ERROR)

        onRemoveStarterPackFeedOk: (starterPackUri, feedUri) => {
            if (feedUri === removingFeed.uri)
                feedListView.model.deleteEntry(removingFeedIndex)
            else
                console.warn("Feed uri mismatch:", feedUri, "removing:", removingFeed.uri, "index:", removingFeedIndex)
        }

        onRemoveStarterPackFeedFailed: (error) => skywalker.showStatusMessage(qsTr(`Failed to remove feed: ${error}`), QEnums.STATUS_LEVEL_ERROR)

        function addFeedToStarterPack(feed) {
            addingFeed = feed
            addStarterPackFeed(starterPack.uri, feed.uri)
        }

        function removeFeedFromStarterPack(feed, index) {
            removingFeed = feed
            removingFeedIndex = index
            removeStarterPackFeed(starterPack.uri, feed.uri)
        }
    }

    ProfileUtils {
        id: profileUtils
        skywalker: page.skywalker

        onProfileViewOk: (profile, listItemUri) => authorListView.model.prependAuthor(profile, listItemUri)
        onProfileViewFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
    }

    function addUser() {
        let component = guiSettings.createComponent("SearchAuthor.qml")
        let searchPage = component.createObject(page, { skywalker: skywalker })
        searchPage.onAuthorClicked.connect((profile) => { // qmllint disable missing-property
            graphUtils.addListUser(starterPack.list.uri, profile)
            root.popStack()
        })
        searchPage.onClosed.connect(() => { root.popStack() })
        root.pushStack(searchPage)
    }

    function addFeed() {
        let component = guiSettings.createComponent("SearchFeed.qml")
        let searchPage = component.createObject(page, { skywalker: skywalker })
        searchPage.onFeedClicked.connect((feed) => { // qmllint disable missing-property
            graphUtils.addFeedToStarterPack(feed)
            root.popStack()
        })
        searchPage.onClosed.connect(() => { root.popStack() })
        root.pushStack(searchPage)
    }

    function copyStarterPackToList() {
        skywalker.showStatusMessage(qsTr("Copying starter pack to list"), QEnums.STATUS_LEVEL_INFO, 30)
        graphUtils.createListFromStarterPack(starterPack)
    }

    Component.onDestruction: {
        skywalker.removePostFeedModel(postFeedModelId)
        skywalker.removeFeedListModel(feedListModelId)
    }

    Component.onCompleted: {
        skywalker.getListFeed(postFeedModelId)

        let feedListModel = skywalker.getFeedListModel(feedListModelId)
        feedListModel.addFeeds(starterPack.feeds)
    }
}
