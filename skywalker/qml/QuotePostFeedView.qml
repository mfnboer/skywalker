import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker
import atproto.lib

SkyPage {
    property string userDid
    property Skywalker skywalker: root.getSkywalker(userDid)
    required property string quoteUri
    required property int topModelId
    property int latestModelId: -1
    readonly property string sideBarTitle: skywalker.getPostFeedModel(topModelId).feedName
    readonly property SvgImage sideBarSvg: SvgFilled.quote
    readonly property int margin: 10

    signal closed

    id: page

    header: PostFeedHeader {
        userDid: page.userDid
        feedName: sideBarTitle
        defaultSvg: sideBarSvg
        visible: !root.showSideBar

        onClosed: page.closed()
    }

    SkyTabBar {
        id: quotesBar
        width: parent.width
        Material.background: guiSettings.backgroundColor
        leftPadding: page.margin
        rightPadding: page.margin

        AccessibleTabButton {
            id: tabTopPosts
            text: qsTr("Top")
            width: implicitWidth;
        }
        AccessibleTabButton {
            id: tabLatestPosts
            text: qsTr("Latest")
            width: implicitWidth;
        }
    }

    Rectangle {
        id: quotesBarSeparator
        anchors.top: quotesBar.bottom
        width: parent.width
        height: 1
        color: guiSettings.separatorColor
    }

    SwipeView {
        id: quotesStack
        anchors.top: quotesBarSeparator.bottom
        anchors.bottom: parent.bottom
        width: parent.width
        currentIndex: quotesBar.currentIndex

        onCurrentIndexChanged: {
            quotesBar.setCurrentIndex(currentIndex)
        }

        SkyListView {
            id: postViewTop
            Layout.preferredWidth: parent.width
            Layout.preferredHeight: parent.height
            cacheBuffer: Screen.height * 3
            model: skywalker.getPostFeedModel(topModelId)
            clip: true
            preloadNextPageFunc: () => skywalker.getQuotesFeedNextPage(topModelId)

            delegate: PostFeedViewDelegate {
                width: postViewTop.width
            }

            FlickableRefresher {
                inProgress: postViewTop.model.getFeedInProgress
                verticalOvershoot: postViewTop.verticalOvershoot
                bottomOvershootFun: () => skywalker.getQuotesFeedNextPage(topModelId)
                enableScrollToTop: true
            }

            EmptyListIndication {
                svg: SvgOutline.noPosts
                text: qsTr("No quotes")
                list: postViewTop
            }

            BusyIndicator {
                anchors.centerIn: parent
                running: postViewTop.model.getFeedInProgress
            }
        }

        SkyListView {
            id: postViewLatest
            Layout.preferredWidth: parent.width
            Layout.preferredHeight: parent.height
            cacheBuffer: Screen.height * 3
            model: latestModelId >= 0 ? skywalker.getPostFeedModel(latestModelId) : null
            clip: true
            preloadNextPageFunc: () => skywalker.getQuotesFeedNextPage(latestModelId)

            SwipeView.onIsCurrentItemChanged: {
                if (latestModelId < 0) {
                    latestModelId = skywalker.createQuotePostFeedModel(quoteUri, SortOrder.LATEST)
                    skywalker.getQuotesFeed(latestModelId)
                }
            }

            delegate: PostFeedViewDelegate {
                width: postViewLatest.width
            }

            FlickableRefresher {
                inProgress: Boolean(postViewLatest.model?.getFeedInProgress)
                verticalOvershoot: postViewLatest.verticalOvershoot
                bottomOvershootFun: () => skywalker.getQuotesFeedNextPage(latestModelId)
                enableScrollToTop: true
            }

            EmptyListIndication {
                svg: SvgOutline.noPosts
                text: qsTr("No quotes")
                list: postViewLatest
            }

            BusyIndicator {
                anchors.centerIn: parent
                running: Boolean(postViewLatest.model?.getFeedInProgress)
            }
        }
    }

    function forceDestroy() {
        if (topModelId !== -1) {
            skywalker.removePostFeedModel(topModelId)
            topModelId = -1
        }

        if (latestModelId !== -1) {
            skywalker.removePostFeedModel(latestModelId)
            latestModelId = -1
        }

        destroy()
    }

    Component.onDestruction: {
        if (topModelId !== -1)
            skywalker.removePostFeedModel(topModelId)

        if (latestModelId !== -1)
            skywalker.removePostFeedModel(latestModelId)
    }
}
