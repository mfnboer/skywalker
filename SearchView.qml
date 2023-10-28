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

                    Keys.onReleased: (event) => {
                        if (event.key === Qt.Key_Return) {
                            page.isTyping = false

                            if (displayText.length > 0) {
                                searchUtils.legacySearchPosts(displayText)
                            }
                        }
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

            Rectangle {
                width: 10
                height: parent.height
                color: "transparent"
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
            text: qsTr("No users found")
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
        }

        ListView {
            id: usersView
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
    }

    GuiSettings {
        id: guiSettigs
    }
}
