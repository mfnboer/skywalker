import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.VirtualKeyboard
import skywalker

Page {
    required property var skywalker
    property var timeline
    property bool isTyping: true

    signal closed

    id: page

    header: Rectangle {
        width: parent.width
        height: guiSettings.headerHeight
        z: guiSettings.headerZLevel
        color: guiSettings.headerColor

        RowLayout {
            width: parent.width
            height: guiSettings.headerHeight

            SvgButton {
                id: backButton
                iconColor: "white"
                Material.background: "transparent"
                svg: svgOutline.arrowBack
                onClicked: page.closed()
            }

            Rectangle {
                radius: 5
                Layout.fillWidth: true
                height: searchText.height
                color: guiSettigs.backgroundColor

                TextInput {
                    id: searchText
                    EnterKeyAction.actionId: EnterKeyAction.Search
                    width: parent.width
                    padding: 5
                    rightPadding: clearButton.width
                    font.pointSize: guiSettings.scaledFont(9/8)

                    onDisplayTextChanged: {
                        page.isTyping = true

                        if (displayText.length > 0) {
                            typeaheadSearchTimer.start()
                        } else {
                            typeaheadSearchTimer.stop()
                            searchUtils.authorTypeaheadList = []
                        }
                    }

                    // Does not work with Android
                    Keys.onReleased: (event) => {
                        if (event.key === Qt.Key_Return) {
                            searchUtils.search(displayText)
                        }
                    }

                    SvgButton {
                        id: clearButton
                        anchors.right: parent.right
                        imageMargin: 8
                        y: parent.y - parent.padding
                        width: height
                        height: parent.height + 10
                        iconColor: guiSettigs.textColor
                        Material.background: "transparent"
                        svg: svgOutline.close
                        visible: searchText.displayText.length > 0
                        onClicked: searchText.clear()
                    }
                }

                Text {
                    width: searchText.width
                    padding: searchText.padding
                    font.pointSize: searchText.font.pointSize
                    color: "grey"
                    text: qsTr("Search")
                    visible: searchText.displayText.length === 0
                }
            }

            // WORKAROUND for Android
            // Qt does not catch the signal of the ENTER key from the Android
            // keyboard, nor can it set the icon to a search icon.
            SvgButton {
                id: searchButton
                iconColor: searchText.displayText.length > 0 ? "white" : "transparent"
                Material.background: "transparent"
                svg: svgOutline.search
                onClicked: searchUtils.search(searchText.displayText)
                enabled: searchText.displayText.length > 0
            }
        }
    }

    footer: SkyFooter {
        timeline: page.timeline
        skywalker: page.skywalker
        searchActive: true
        onHomeClicked: root.viewTimeline()
        onNotificationsClicked: root.viewNotifications()
    }

    AuthorTypeaheadListView {
        id: typeaheadView
        anchors.fill: parent
        model: searchUtils.authorTypeaheadList
        visible: page.isTyping

        Text {
            topPadding: 10
            anchors.horizontalCenter: parent.horizontalCenter
            color: "grey"
            elide: Text.ElideRight
            text: qsTr("No matching user name found")
            visible: typeaheadView.count === 0
        }
    }

    TabBar {
        id: searchResultsBar
        width: parent.width
        visible: !page.isTyping

        TabButton {
            text: qsTr("Posts")
        }
        TabButton {
            text: qsTr("Users")
        }
    }

    StackLayout {
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
            ScrollIndicator.vertical: ScrollIndicator {}

            delegate: PostFeedViewDelegate {
                viewWidth: postsView.width
            }

            Text {
                topPadding: 10
                anchors.horizontalCenter: parent.horizontalCenter
                color: "grey"
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
            ScrollIndicator.vertical: ScrollIndicator {}

            delegate: AuthorViewDelegate {
                viewWidth: postsView.width
                onFollow: (profile) => { graphUtils.follow(profile) }
                onUnfollow: (did, uri) => { graphUtils.unfollow(did, uri) }
            }

            Text {
                topPadding: 10
                anchors.horizontalCenter: parent.horizontalCenter
                color: "grey"
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
            if (searchText.displayText.length > 0)
                searchUtils.searchAuthorsTypeahead(searchText.displayText)
        }
    }

    SearchUtils {
        id: searchUtils
        skywalker: page.skywalker

        function search(query) {
            page.isTyping = false

            if (query.length > 0) {
                searchUtils.legacySearchPosts(query)
                searchUtils.legacySearchActors(query)
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
        id: guiSettigs
    }
}
