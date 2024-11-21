import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

SkyPage {
    required property var skywalker
    property int unreadPosts: viewStack.children[viewStack.currentIndex].unreadPosts
    property int margin: 10

    id: page

    Accessible.role: Accessible.Pane
    Accessible.name: skywalker.timelineModel.feedName

    onCover: {
        timelineView.cover()
    }

    header: PostFeedHeader {
        skywalker: page.skywalker
        feedName: skywalker.timelineModel.feedName
        showAsHome: true
        isHomeFeed: true
    }

    footer: SkyFooter {
        timeline: page
        skywalker: page.skywalker
        homeActive: true
        onHomeClicked: getCurrentView().moveToHome()
        onNotificationsClicked: root.viewNotifications()
        onSearchClicked: root.viewSearchView()
        onFeedsClicked: root.viewFeedsView()
        onMessagesClicked: root.viewChat()
    }

    TabBar {
        id: viewBar
        z: guiSettings.headerZLevel
        width: parent.width
        Material.background: guiSettings.backgroundColor
        leftPadding: page.margin
        rightPadding: page.margin

        AccessibleTabButton {
            id: tabTimeline
            text: qsTr(`Full feed (${timelineView.unreadPosts}/${timelineView.count})`)
            width: implicitWidth;
        }
        AccessibleTabButton {
            id: tabTest
            text: skywalker.testTimelineViewModel ? `${skywalker.testTimelineViewModel.feedName} (${testView.unreadPosts}/${testView.count})` : ""
            width: implicitWidth;
        }
    }

    Rectangle {
        id: viewBarSeparator
        z: guiSettings.headerZLevel
        anchors.top: viewBar.bottom
        width: parent.width
        height: 1
        color: guiSettings.separatorColor
    }

    StackLayout {
        id: viewStack
        anchors.top: viewBarSeparator.bottom
        anchors.bottom: parent.bottom
        width: parent.width
        currentIndex: viewBar.currentIndex

        TimelineView {
            id: timelineView
            Layout.preferredWidth: viewStack.width
            Layout.preferredHeight: viewStack.height
            skywalker: page.skywalker
        }
        TimelineView {
            id: testView
            inSync: false
            isView: true
            model: skywalker.testTimelineViewModel
            Layout.preferredWidth: viewStack.width
            Layout.preferredHeight: viewStack.height
            skywalker: page.skywalker
        }
    }

    function getCurrentView() {
        return viewStack.children[viewStack.currentIndex]
    }

    function setInSync(index) {
        timelineView.setInSync(index)
        testView.setInSync(-1)
    }

    function stopSync() {
        timelineView.stopSync()
    }

    function moveToPost(index) {
        timelineView.moveToPost(index)
    }
}
