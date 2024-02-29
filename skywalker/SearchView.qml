import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Page {
    required property var skywalker
    property var timeline
    property bool isTyping: true
    property string initialSearch

    signal closed

    id: page
    clip: true

    Accessible.role: Accessible.Pane

    header: SearchHeader {
        onBack: page.closed()

        onSearchTextChanged: (text) => {
            page.isTyping = true

            if (text.length > 0) {
                typeaheadSearchTimer.start()
            } else {
                typeaheadSearchTimer.stop()
                searchUtils.authorTypeaheadList = []
            }
        }

        onKeyRelease: (event) => {
            if (event.key === Qt.Key_Return) {
                searchUtils.search(getDisplayText())
            }
        }

        onSearch: (text) => { searchUtils.search(text) }
    }

    footer: SkyFooter {
        timeline: page.timeline
        skywalker: page.skywalker
        searchActive: true
        onHomeClicked: root.viewTimeline()
        onNotificationsClicked: root.viewNotifications()
        onFeedsClicked: root.viewFeedsView()
    }

    SimpleAuthorListView {
        id: typeaheadView
        anchors.fill: parent
        model: searchUtils.authorTypeaheadList
        visible: page.isTyping

        onAuthorClicked: (profile) => { page.skywalker.getDetailedProfile(profile.did) }

        AccessibleText {
            topPadding: 10
            anchors.horizontalCenter: parent.horizontalCenter
            color: Material.color(Material.Grey)
            elide: Text.ElideRight
            text: qsTr("No matching user name found")
            visible: typeaheadView.count === 0
        }
    }

    TabBar {
        id: searchResultsBar
        width: parent.width
        visible: !page.isTyping

        AccessibleTabButton {
            text: qsTr("Posts")
        }
        AccessibleTabButton {
            text: qsTr("Users")
        }
    }

    StackLayout {
        id: searchStack
        anchors.top: searchResultsBar.bottom
        anchors.bottom: parent.bottom
        width: parent.width
        currentIndex: searchResultsBar.currentIndex
        visible: !page.isTyping

        ListView {
            id: postsView
            width: parent.width
            height: parent.height
            spacing: 0
            clip: true
            model: searchUtils.getSearchPostFeedModel()
            flickDeceleration: guiSettings.flickDeceleration
            ScrollIndicator.vertical: ScrollIndicator {}

            Accessible.role: Accessible.List

            delegate: PostFeedViewDelegate {
                viewWidth: postsView.width
            }

            FlickableRefresher {
                inProgress: searchUtils.searchPostsInProgress
                verticalOvershoot: postsView.verticalOvershoot
                bottomOvershootFun: () => searchUtils.getNextPageSearchPosts(header.getDisplayText())
                topText: ""
            }

            AccessibleText {
                topPadding: 10
                anchors.horizontalCenter: parent.horizontalCenter
                color: Material.color(Material.Grey)
                elide: Text.ElideRight
                text: qsTr("No posts found")
                visible: postsView.count === 0 && !searchUtils.searchPostsInProgress
            }

            BusyIndicator {
                anchors.centerIn: parent
                running: searchUtils.searchPostsInProgress
            }
        }

        ListView {
            id: usersView
            width: parent.width
            height: parent.height
            spacing: 0
            clip: true
            model: searchUtils.getSearchUsersModel()
            flickDeceleration: guiSettings.flickDeceleration
            ScrollIndicator.vertical: ScrollIndicator {}

            Accessible.role: Accessible.List

            delegate: AuthorViewDelegate {
                viewWidth: postsView.width
                onFollow: (profile) => { graphUtils.follow(profile) }
                onUnfollow: (did, uri) => { graphUtils.unfollow(did, uri) }
            }

            FlickableRefresher {
                inProgress: searchUtils.searchActorsInProgress
                verticalOvershoot: usersView.verticalOvershoot
                bottomOvershootFun: () => searchUtils.getNextPageSearchActors(header.getDisplayText())
                topText: ""
            }

            AccessibleText {
                topPadding: 10
                anchors.horizontalCenter: parent.horizontalCenter
                color: Material.color(Material.Grey)
                elide: Text.ElideRight
                text: qsTr("No users found")
                visible: usersView.count === 0 && !searchUtils.searchActorsInProgress
            }

            BusyIndicator {
                anchors.centerIn: parent
                running: searchUtils.searchActorsInProgress
            }
        }
    }

    Timer {
        id: typeaheadSearchTimer
        interval: 500
        onTriggered: {
            const text = page.header.getDisplayText()

            if (text.length > 0)
                searchUtils.searchAuthorsTypeahead(text)
        }
    }

    SearchUtils {
        id: searchUtils
        skywalker: page.skywalker

        function search(query) {
            page.isTyping = false

            if (query.length > 0) {
                searchUtils.searchPosts(query)
                searchUtils.searchActors(query)
            }
        }

        Component.onDestruction: {
            // The destuctor of SearchUtils is called too late by the QML engine
            // Remove models now before the Skywalker object is destroyed.
            searchUtils.removeModels()
        }
    }

    GraphUtils {
        id: graphUtils
        skywalker: page.skywalker

        onFollowFailed: (error) => { statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR) }
        onUnfollowFailed: (error) => { statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR) }
    }

    GuiSettings {
        id: guiSettings
    }

    function forceDestroy() {
        searchUtils.clearAllSearchResults();
        usersView.model = null
        postsView.model = null
        searchUtils.removeModels()
        destroy()
    }

    function hide() {
        page.header.unfocus()
    }

    function show(searchText = "") {
        page.header.forceFocus()
        initialSearch = searchText

        if (searchText) {
            header.setSearchText(searchText)
            searchUtils.search(searchText)
        }
    }

    Component.onCompleted: {
        if (initialSearch)
            searchUtils.search(searchText)
    }
}
