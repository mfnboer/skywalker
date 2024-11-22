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
        showMoreOptions: true

        onAddUserView: page.addUserView()
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
            implicitHeight: 30
            text: qsTr("Full feed")
            width: implicitWidth;
        }

        function addTab(name) {
            let component = Qt.createComponent("SkyTabWithCloseButton.qml")
            let tab = component.createObject(viewBar, { implicitHeight: 30, text: name })
            tab.onClosed.connect(() => page.closeView(tab))
            addItem(tab)
            setCurrentIndex(count - 1)
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

        Repeater {
            model: skywalker.timelineModel.filteredPostFeedModels

            delegate: TimelineView {
                required property var modelData

                Layout.preferredWidth: viewStack.width
                Layout.preferredHeight: viewStack.height
                skywalker: page.skywalker
                isView: true
                model: modelData
            }
        }

        function addTimelineView(filteredPostFeedModel) {
            viewBar.addTab(filteredPostFeedModel.feedName)
            children[count - 1].setInSync(-1)
        }
    }

    function getCurrentView() {
        return viewStack.children[viewStack.currentIndex]
    }

    function setInSync(index) {
        timelineView.setInSync(index)
    }

    function stopSync() {
        timelineView.stopSync()
    }

    function moveToPost(index) {
        timelineView.moveToPost(index)
    }

    function addUserView() {
        let component = Qt.createComponent("AddUserTimelineView.qml")
        let addViewPage = component.createObject(page, { skywalker: skywalker })
        addViewPage.onSelected.connect((profile) => { // qmllint disable missing-property
                page.showUserView(profile)
                root.popStack()
        })
        addViewPage.onClosed.connect(() => { root.popStack() }) // qmllint disable missing-property
        pushStack(addViewPage)
    }

    function showUserView(profile) {
        let authorPostFilterModel = skywalker.timelineModel.addAuthorFilter(profile.did, profile.handle)
        viewStack.addTimelineView(authorPostFilterModel)
    }

    function closeView(tab) {
        const index = tab.TabBar.index

        if (viewBar.currentIndex === index)
            viewBar.setCurrentIndex(index - 1)

        let view = skywalker.timelineModel.filteredPostFeedModels[index - 1]
        skywalker.timelineModel.deleteFilteredPostFeedModel(view)
        viewBar.removeItem(tab)
    }
}
