import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyPage {

    required property var skywalker
    property var timeline

    signal closed

    id: page
    clip: true

    Accessible.role: Accessible.Pane

    header: SearchHeader {
        placeHolderText: qsTr("Search feeds")

        onBack: page.closed()

        onSearchTextChanged: (text) => {
            typeaheadSearchTimer.start()
        }

        onSearch: (text) => {
            typeaheadSearchTimer.stop()
            searchFeeds()
        }
    }

    // Place footer explicitly on the bottom instead of using Page.footer
    // This way the content goes all the way to the bottom underneath the footer.
    SkyFooter {
        id: pageFooter
        width: parent.width
        anchors.bottom: parent.bottom
        timeline: page.timeline
        skywalker: page.skywalker
        feedsActive: true
        onHomeClicked: root.viewTimeline()
        onSearchClicked: root.viewSearchView()
        onNotificationsClicked: root.viewNotifications()
        onFeedsClicked: feedListView.positionViewAtBeginning()
        onMessagesClicked: root.viewChat()
    }

    SkyTabBar {
        id: feedsBar
        width: parent.width

        AccessibleTabButton {
            text: qsTr("Search results")
        }
        AccessibleTabButton {
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
            Layout.preferredWidth: parent.width
            Layout.preferredHeight: parent.height
            spacing: 0
            clip: true
            model: searchUtils.getSearchFeedsModel()
            flickDeceleration: guiSettings.flickDeceleration
            maximumFlickVelocity: guiSettings.maxFlickVelocity
            pixelAligned: guiSettings.flickPixelAligned
            ScrollIndicator.vertical: ScrollIndicator {}

            Accessible.role: Accessible.List

            delegate: GeneratorViewDelegate {
                width: page.width
            }

            FlickableRefresher {
                scrollToTopButtonMargin: pageFooter.height
                inProgress: searchUtils.searchFeedsInProgress
                topOvershootFun: () => searchUtils.searchFeeds(page.header.getDisplayText()) // qmllint disable missing-property
                bottomOvershootFun: () => searchUtils.getNextPageSearchFeeds(page.header.getDisplayText()) // qmllint disable missing-property
                topText: qsTr("Pull down to refresh")
            }

            EmptyListIndication {
                svg: SvgOutline.noPosts
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
            Layout.preferredWidth: parent.width
            Layout.preferredHeight: parent.height
            spacing: 0
            clip: true
            model: skywalker.favoriteFeeds.getSavedFeedsModel()
            boundsBehavior: Flickable.StopAtBounds
            flickDeceleration: guiSettings.flickDeceleration
            maximumFlickVelocity: guiSettings.maxFlickVelocity
            pixelAligned: guiSettings.flickPixelAligned
            ScrollIndicator.vertical: ScrollIndicator {}

            Accessible.role: Accessible.List

            delegate: GeneratorViewDelegate {
                width: page.width

            }

            FlickableRefresher {
                scrollToTopButtonMargin: pageFooter.height
            }

            EmptyListIndication {
                svg: SvgOutline.noPosts
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
        skywalker: page.skywalker // qmllint disable missing-type

        Component.onDestruction: {
            // The destuctor of SearchUtils is called too late by the QML engine
            // Remove models now before the Skywalker object is destroyed.
            searchUtils.removeModels()
            skywalker.favoriteFeeds.removeSavedFeedsModel()
        }
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
        const text = page.header.getDisplayText(page.header.getDisplayText()) // qmllint disable missing-property
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
        page.header.unfocus() // qmllint disable missing-property
    }

    function show() {
        page.header.forceFocus() // qmllint disable missing-property
    }

    Component.onCompleted: {
        searchFeeds()
    }
}
