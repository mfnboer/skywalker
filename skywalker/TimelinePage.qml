import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

SkyPage {
    required property var skywalker
    property int unreadPosts: viewStack.currentIndex >= 0 ? viewStack.children[viewStack.currentIndex].unreadPosts : 0
    property int margin: 10

    id: page

    Accessible.role: Accessible.Pane
    Accessible.name: skywalker.timelineModel.feedName

    onCover: {
        viewStack.cover()
    }

    header: PostFeedHeader {
        skywalker: page.skywalker
        feedName: skywalker.timelineModel.feedName
        showAsHome: true
        isHomeFeed: true
        showMoreOptions: true

        onAddUserView: page.addUserView()
        onAddHashtagView: page.addHashtagView()
        onAddFocusHashtagView: page.addFocusHashtagView()
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
        property int numDots: 0

        id: viewBar
        z: guiSettings.headerZLevel
        width: parent.width
        contentHeight: 40
        Material.background: guiSettings.backgroundColor
        visible: count > 1

        onCurrentItemChanged: currentItem.showDot = false

        SkyTabWithCloseButton {
            id: tabTimeline
            text: qsTr("Full feed")
            showCloseButton: false
            onShowDotChanged: viewBar.numDots += showDot ? 1 : -1
        }

        function addTab(name, backgroundColor, profile) {
            let component = Qt.createComponent("SkyTabWithCloseButton.qml")
            let tab = component.createObject(viewBar, {
                                                 text: name,
                                                 backgroundColor: backgroundColor,
                                                 profile: profile })
            tab.onShowDotChanged.connect(() => viewBar.numDots += tab.showDot ? 1 : -1)
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
        height: visible ? 1 : 0
        color: viewBar.numDots > 0 ? guiSettings.accentColor : guiSettings.separatorColor
        visible: viewBar.visible
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

            onNewPosts: {
                if (!StackLayout.isCurrentItem)
                    tabTimeline.showDot = true
            }

            StackLayout.onIsCurrentItemChanged: {
                if (!StackLayout.isCurrentItem)
                    cover()
            }
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

                onNewPosts: {
                    if (!StackLayout.isCurrentItem) {
                        viewBar.itemAt(StackLayout.index).showDot = true
                    }
                }

                StackLayout.onIsCurrentItemChanged: {
                    if (!StackLayout.isCurrentItem)
                        cover()
                }
            }
        }

        function addTimelineView(filteredPostFeedModel) {
            viewBar.addTab(filteredPostFeedModel.feedName,
                           filteredPostFeedModel.backgroundColor,
                           filteredPostFeedModel.profile)
            children[count - 1].setInSync(-1)
        }

        function cover() {
            let item = children[currentIndex]

            if (item)
                item.cover()
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

    function addHashtagView() {
        let component = Qt.createComponent("AddHashtagTimelineView.qml")
        let addViewPage = component.createObject(page, { skywalker: skywalker })
        addViewPage.onSelected.connect((hashtag) => { // qmllint disable missing-property
                page.showHashtagView(hashtag)
                root.popStack()
        })
        addViewPage.onClosed.connect(() => { root.popStack() }) // qmllint disable missing-property
        pushStack(addViewPage)
    }

    function addFocusHashtagView() {
        let component = Qt.createComponent("AddFocusHashtagTimelineView.qml")
        let addViewPage = component.createObject(page, { skywalker: skywalker })
        addViewPage.onSelected.connect((focusHashtagEntry) => { // qmllint disable missing-property
                page.showFocusHashtagView(focusHashtagEntry)
                root.popStack()
        })
        addViewPage.onClosed.connect(() => { root.popStack() }) // qmllint disable missing-property
        pushStack(addViewPage)
    }

    function showUserView(profile) {
        let postFilterModel = skywalker.timelineModel.addAuthorFilter(profile)
        viewStack.addTimelineView(postFilterModel)
    }

    function showHashtagView(hashtag) {
        let postFilterModel = skywalker.timelineModel.addHashtagFilter(hashtag)
        viewStack.addTimelineView(postFilterModel)
    }

    function showFocusHashtagView(focusHashtagEntry) {
        let postFilterModel = skywalker.timelineModel.addFocusHashtagFilter(focusHashtagEntry)
        viewStack.addTimelineView(postFilterModel)
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
