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

    ListView {
        id: feedListView
        anchors.fill: parent
        spacing: 0
        model: searchUtils.getSearchFeedsModel()
        flickDeceleration: guiSettings.flickDeceleration
        ScrollIndicator.vertical: ScrollIndicator {}

        delegate: GeneratorViewDelegate {
            viewWidth: page.width

            onFeedClicked: (feed) => {
                const modelId = skywalker.createPostFeedModel(feed)
                skywalker.getFeed(modelId)
                let component = Qt.createComponent("PostFeedView.qml")
                let view = component.createObject(page, { skywalker: skywalker, modelId: modelId })
                view.onClosed.connect(() => root.popStack())
                root.pushStack(view)
            }
        }

        FlickableRefresher {
            inProgress: searchUtils.searchFeedsInProgress
            verticalOvershoot: feedListView.verticalOvershoot
            bottomOvershootFun: () => searchUtils.getNextPageSearchFeeds(page.header.getDisplayText())
            topText: ""
        }
    }

    BusyIndicator {
        anchors.centerIn: parent
        running: searchUtils.searchFeedsInProgress
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
        }
    }

    GuiSettings {
        id: guiSettings
    }

    function searchFeeds() {
        feedListView.positionViewAtBeginning()
        const text = page.header.getDisplayText(page.header.getDisplayText())
        searchUtils.searchFeeds(text)
    }

    function forceDestroy() {
        searchUtils.clearAllSearchResults();
        feedListView.model = null
        searchUtils.removeModels()
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
