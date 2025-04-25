import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyPage {
    required property starterpackview starterPack
    property var skywalker: root.getSkywalker()
    readonly property int postFeedModelId: skywalker.createPostFeedModel(starterPack.list)
    readonly property int feedListModelId: skywalker.createFeedListModel()
    readonly property int margin: 10
    readonly property string sideBarTitle: qsTr("Starter pack")
    readonly property SvgImage sideBarSvg: SvgOutline.starterpack

    signal closed

    id: page

    header: SimpleHeader {
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
            skywalker: page.skywalker
            modelId: skywalker.createAuthorListModel(QEnums.AUTHOR_LIST_LIST_MEMBERS, starterPack.list.uri)
            listUri: starterPack.list.uri
            clip: true

            header: PlaceholderHeader { height: feedStack.headerHeight }
            headerPositioning: ListView.InlineHeader
            footer: PlaceholderHeader { height: page.height}
            footerPositioning: ListView.InlineFooter

            Component.onCompleted: skywalker.getAuthorList(modelId)
        }

        SkyListView {
            id: feedListView
            model: skywalker.getFeedListModel(feedListModelId)
            boundsBehavior: Flickable.StopAtBounds
            clip: true

            header: PlaceholderHeader { height: feedStack.headerHeight }
            headerPositioning: ListView.InlineHeader
            footer: PlaceholderHeader { height: page.height}
            footerPositioning: ListView.InlineFooter

            delegate: GeneratorViewDelegate {
                width: feedListView.width

                onHideFollowing: (feed, hide) => feedUtils.hideFollowing(feed.uri, hide)
            }

            EmptyListIndication {
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
            footer: PlaceholderHeader { height: page.height }
            footerPositioning: ListView.InlineFooter

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
            if (starterPack.feeds.length === 0)
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
            columns: 2
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

            SkyCleanedTextLine {
                topPadding: 10
                Layout.fillWidth: true
                elide: Text.ElideRight
                font.bold: true
                color: guiSettings.textColor
                plainText: starterPack.name
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
            contentLabels: starterPack.labels
            contentAuthorDid: starterPack.creator.did
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
            if (starterPack.feeds.length === 0)
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
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.rightMargin: page.margin
        svg: SvgOutline.moreVert
        accessibleName: qsTr("more options")
        onClicked: moreMenu.open()

        Menu {
            id: moreMenu
            modal: true

            CloseMenuItem {
                text: qsTr("<b>Starter pack</b>")
                Accessible.name: qsTr("close more options menu")
            }
            AccessibleMenuItem {
                text: qsTr("Translate")
                enabled: starterPack.description
                onTriggered: root.translateText(starterPack.description)

                MenuItemSvg { svg: SvgOutline.googleTranslate }
            }
            AccessibleMenuItem {
                text: qsTr("Share")
                onTriggered: skywalker.shareStarterPack(starterPack)

                MenuItemSvg { svg: SvgOutline.share }
            }
            AccessibleMenuItem {
                text: qsTr("Copy to list")
                onTriggered: copyStarterPackToList()

                MenuItemSvg { svg: SvgOutline.list }
            }
            AccessibleMenuItem {
                text: qsTr("Report starter pack")
                onTriggered: root.reportStarterPack(starterPack)

                MenuItemSvg { svg: SvgOutline.report }
            }
            AccessibleMenuItem {
                text: qsTr("Emoji names")
                visible: UnicodeFonts.hasEmoji(starterPack.description)
                onTriggered: root.showEmojiNamesList(starterPack.description)

                MenuItemSvg { svg: SvgOutline.smiley }
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
        id: graphUtils
        skywalker: page.skywalker

        onCreatedListFromStarterPackOk: (pack, listUri, listCid) => {
            skywalker.showStatusMessage(qsTr("List created"), QEnums.STATUS_LEVEL_INFO)
            getListViewTimer.go(listUri)
        }

        onCreatedListFromStarterPackFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)

        onGetListOk: (list) => root.viewListFeedDescription(list)
        onGetListFailed: (error) => {
            // The network may take a while before you can retrieve a new list.
            console.warn(error)
            skywalker.showStatusMessage(qsTr("You can find the new list in your overview of user lists"), QEnums.STATUS_LEVEL_INFO)
        }
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
