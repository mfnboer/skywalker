import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Page {

    required property var skywalker
    property var timeline

    signal closed

    id: page

    header: SearchHeader {
        placeHolderText: qsTr("Search feeds")

        onBack: page.closed()

        onSearchTextChanged: (text) => {
            typeaheadSearchTimer.start()
        }

        onKeyRelease: (event) => {
            if (event.key === Qt.Key_Return) {
                searchFeeds()
            }
        }

        onSearch: (text) => {
            typeaheadSearchTimer.stop()
            searchFeeds()
        }
    }

    footer: SkyFooter {
        timeline: page.timeline
        skywalker: page.skywalker
        feedsActive: true
        onHomeClicked: root.viewTimeline()
        onSearchClicked: root.viewSearchView()
        onNotificationsClicked: root.viewNotifications()
        onFeedsClicked: feedListView.positionViewAtBeginning()
    }

    TabBar {
        id: feedsBar
        width: parent.width

        TabButton {
            text: qsTr("Search results")
        }
        TabButton {
            text: qsTr("Saved feeds")
        }
    }

    StackLayout {
        anchors.top: feedsBar.bottom
        anchors.bottom: parent.bottom
        width: parent.width
        currentIndex: feedsBar.currentIndex

        ListView {
            id: feedListView
            width: parent.width
            height: parent.height
            spacing: 0
            clip: true
            model: searchUtils.getSearchFeedsModel()
            flickDeceleration: guiSettings.flickDeceleration
            ScrollIndicator.vertical: ScrollIndicator {}

            delegate: GeneratorViewDelegate {
                viewWidth: page.width

                onFeedClicked: (feed) => root.viewPostFeed(feed)
                onAddClicked: (feed, add) => saveFeed(feed, add)
                onFavoriteClicked: (feed, add) => pinFeed(feed, add)
            }

            FlickableRefresher {
                inProgress: searchUtils.searchFeedsInProgress
                verticalOvershoot: feedListView.verticalOvershoot
                bottomOvershootFun: () => searchUtils.getNextPageSearchFeeds(page.header.getDisplayText())
                topText: ""
            }

            EmptyListIndication {
                svg: svgOutline.noPosts
                text: qsTr("No results")
                list: feedListView
            }

            BusyIndicator {
                anchors.centerIn: parent
                running: searchUtils.searchFeedsInProgress
            }
        }

        ListView {
            id: savedFeedsView
            width: parent.width
            height: parent.height
            spacing: 0
            clip: true
            model: skywalker.favoriteFeeds.getSavedFeedsModel()
            boundsBehavior: Flickable.StopAtBounds
            flickDeceleration: guiSettings.flickDeceleration
            ScrollIndicator.vertical: ScrollIndicator {}

            delegate: GeneratorViewDelegate {
                viewWidth: page.width

                onFeedClicked: (feed) => root.viewPostFeed(feed)
                onAddClicked: (feed, add) => saveFeed(feed, add)
                onFavoriteClicked: (feed, add) => pinFeed(feed, add)
            }

            EmptyListIndication {
                svg: svgOutline.noPosts
                text: qsTr("No saved feeds")
                list: savedFeedsView
            }

            BusyIndicator {
                anchors.centerIn: parent
                running: skywalker.favoriteFeeds.updateSavedFeedsModelInProgress
            }
        }
    }

    Timer {
        id: typeaheadSearchTimer
        interval: 1000
        onTriggered: searchFeeds()
    }

    SearchUtils {
        id: searchUtils
        skywalker: page.skywalker

        Component.onDestruction: {
            // The destuctor of SearchUtils is called too late by the QML engine
            // Remove models now before the Skywalker object is destroyed.
            searchUtils.removeModels()
            skywalker.favoriteFeeds.removeSavedFeedsModel()
        }
    }

    GuiSettings {
        id: guiSettings
    }

    function saveFeed(feed, save) {
        if (save)
            skywalker.favoriteFeeds.addFeed(feed)
        else
            skywalker.favoriteFeeds.removeFeed(feed)

        skywalker.saveFavoriteFeeds()
    }

    function pinFeed(feed, pin) {
        skywalker.favoriteFeeds.pinFeed(feed, pin)
        skywalker.saveFavoriteFeeds()
    }

    function searchFeeds() {
        feedListView.positionViewAtBeginning()
        const text = page.header.getDisplayText(page.header.getDisplayText())
        searchUtils.searchFeeds(text)
    }

    function forceDestroy() {
        searchUtils.clearAllSearchResults()
        feedListView.model = null
        searchUtils.removeModels()
        savedFeedsView.model = null
        skywalker.favoriteFeeds.removeSavedFeedsModel()
        destroy()
    }

    function hide() {
        page.header.unfocus()
    }

    function show() {
        page.header.forceFocus()
    }

    Component.onCompleted: {
        searchFeeds()
    }
}
