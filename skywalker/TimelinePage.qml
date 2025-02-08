import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

SkyPage {
    required property var skywalker
    property var currentViewItem: viewStack.currentIndex >= 0 ? viewStack.children[viewStack.currentIndex] : null
    property int unreadPosts: (currentViewItem && currentViewItem instanceof TimelineView) ? currentViewItem.unreadPosts : 0
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
        onAddMediaView: page.showMediaView()
        onAddVideoView: page.showVideoView()
    }

    footer: SkyFooter {
        timeline: page
        skywalker: page.skywalker
        homeActive: true
        onHomeClicked: currentViewItem.moveToHome()
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

        function updateTab(index, name, backgroundColor, profile) {
            let item = itemAt(index)

            if (!item) {
                console.warn("Item does not exist:", index, name)
                return
            }

            item.text = name
            item.backgroundColor = backgroundColor
            item.profile = profile
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
        property list<var> syncBackup: []

        id: viewStack
        anchors.top: viewBar.visible ? viewBarSeparator.bottom : parent.top
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

        // When the model of a repeater changes then all items are destroyed
        // and created again. To keep position in those views, we backup the sync
        // information before the change, and restore after.

        function backupViewSync(skipIndex = -1) {
            syncBackup = []

            for (let i = 1; i < count; ++i) {
                if (i == skipIndex)
                    continue

                const view = children[i]
                const syncIndex = view.getLastVisibleIndex()
                const syncOffset = view.calcVisibleOffsetY(syncIndex)
                syncBackup.push([syncIndex, syncOffset])
            }
        }

        function restoreViewSync() {
            for (let i = 0; i < syncBackup.length; ++i) {
                let view = children[i + 1]
                const syncIndex = syncBackup[i][0]
                const syncOffset = syncBackup[i][1]
                view.setInSync(syncIndex, syncOffset)
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

    function setInSync(index, offsetY = 0) {
        timelineView.setInSync(index, offsetY)
    }

    function stopSync() {
        timelineView.stopSync()
    }

    function moveToPost(index) {
        timelineView.moveToPost(index)
    }

    function resumeTimeline(index, offsetY) {
        timelineView.resumeTimeline(index, offsetY)
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
        skywalker.timelineModel.addAuthorFilter(profile)
    }

    function showHashtagView(hashtag) {
        skywalker.timelineModel.addHashtagFilter(hashtag)
    }

    function showFocusHashtagView(focusHashtagEntry) {
        skywalker.timelineModel.addFocusHashtagFilter(focusHashtagEntry)
    }

    function showVideoView() {
        skywalker.timelineModel.addVideoFilter()
    }

    function showMediaView() {
        skywalker.timelineModel.addMediaFilter()
    }

    function switchToFullView() {
        viewBar.setCurrentIndex(0)
    }

    function closeView(tab) {
        const index = tab.TabBar.index
        let filterModel = skywalker.timelineModel.filteredPostFeedModels[index - 1]
        skywalker.timelineModel.deleteFilteredPostFeedModel(filterModel)
    }

    function filteredPostFeedModelAboutToBeAddedHandler() {
        viewStack.backupViewSync()
    }

    function filteredPostFeedModelAdded(filterModel) {
        viewStack.addTimelineView(filterModel)
        viewStack.restoreViewSync()
    }

    function filteredPostFeedModelAboutToBeDeleted(index) {
        const viewIndex = index + 1 // first view has index 1
        viewStack.backupViewSync(viewIndex)
        viewBar.setCurrentIndex(0)
        viewBar.takeItem(viewIndex)
    }

    function filteredPostFeedModelDeleted(index) {
        viewStack.restoreViewSync()
    }

    function filteredPostFeedModelUpdated(index) {
        console.debug("UPDATED:", index)
        const filter = skywalker.timelineModel.filteredPostFeedModels[index]
        viewBar.updateTab(index + 1, filter.feedName, filter.backgroundColor, filter.profile)
    }

    Component.onDestruction: {
        skywalker.timelineModel.onFilteredPostFeedModelAboutToBeAdded.disconnect(filteredPostFeedModelAboutToBeAddedHandler)
        skywalker.timelineModel.onFilteredPostFeedModelAdded.disconnect(filteredPostFeedModelAdded)
        skywalker.timelineModel.onFilteredPostFeedModelAboutToBeDeleted.disconnect(filteredPostFeedModelAboutToBeDeleted)
        skywalker.timelineModel.onFilteredPostFeedModelDeleted.disconnect(filteredPostFeedModelDeleted)
        skywalker.timelineModel.onFilteredPostFeedModelUpdated.disconnect(filteredPostFeedModelUpdated)
    }

    Component.onCompleted: {
        skywalker.timelineModel.onFilteredPostFeedModelAboutToBeAdded.connect(filteredPostFeedModelAboutToBeAddedHandler)
        skywalker.timelineModel.onFilteredPostFeedModelAdded.connect(filteredPostFeedModelAdded)
        skywalker.timelineModel.onFilteredPostFeedModelAboutToBeDeleted.connect(filteredPostFeedModelAboutToBeDeleted)
        skywalker.timelineModel.onFilteredPostFeedModelDeleted.connect(filteredPostFeedModelDeleted)
        skywalker.timelineModel.onFilteredPostFeedModelUpdated.connect(filteredPostFeedModelUpdated)
    }
}
