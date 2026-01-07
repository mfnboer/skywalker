import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

SkyPage {
    required property var skywalker
    property int footerHeight: root.footer.height
    property var currentViewItem: viewStack.currentIndex >= 0 ? viewStack.children[viewStack.currentIndex] : null
    property int unreadPosts: (currentViewItem && currentViewItem instanceof TimelineView) ? currentViewItem.unreadPosts : 0
    property int margin: 10
    property var userSettings: skywalker.getUserSettings()
    readonly property int favoritesY: (currentViewItem && currentViewItem.favoritesY !== 'undefined') ? currentViewItem.favoritesY : 0
    readonly property int extraFooterMargin: viewBar.visible && viewBar.position == TabBar.Footer ? viewBar.height : 0

    id: page

    Accessible.role: Accessible.Pane
    Accessible.name: skywalker.timelineModel.feedName

    onCover: {
        viewStack.cover()
    }

    StackLayout {
        property list<var> syncBackup: []

        id: viewStack
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: parent.width
        currentIndex: 0

        TimelineView {
            id: timelineView
            Layout.preferredWidth: viewStack.width
            Layout.preferredHeight: viewStack.height
            headerMargin: viewBar.visible && viewBar.position == TabBar.Header ? viewBar.height : 0

            skywalker: page.skywalker

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
                headerMargin: viewBar.position == TabBar.Header ? viewBar.height : 0

                skywalker: page.skywalker
                isView: true
                model: modelData

                onUnreadPostsChanged: {
                    let item = viewBar.itemAt(StackLayout.index)

                    if (item)
                        item.counter = unreadPosts
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
                if (i === skipIndex)
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

    SkyTabBar {
        id: viewBar
        y: (position == TabBar.Header && currentViewItem && typeof currentViewItem.visibleHeaderHeight !== 'undefined') ? currentViewItem.visibleHeaderHeight : parent.height - height - footerHeight
        z: guiSettings.headerZLevel
        width: parent.width
        position: userSettings.favoritesBarPosition === QEnums.FAVORITES_BAR_POSITION_TOP ? TabBar.Footer : TabBar.Header
        Material.background: guiSettings.backgroundColor
        visible: count > 2

        onCurrentIndexChanged: {
            if (currentIndex < 0)
                return

            if (currentIndex === count - 1)
                Qt.callLater(() => setCurrentIndex(0))
            else
                viewStack.currentIndex = currentIndex
        }

        SkyTabWithCloseButton {
            id: tabTimeline
            text: qsTr("Full feed")
            counter: timelineView.unreadPosts
            showCloseButton: false

            onPressAndHold: showTimelineViewsSorter()
        }

        SkySettingsTabButton {
            visible: skywalker.timelineModel.filteredPostFeedModels.length > 1
            onActivated: showTimelineViewsSorter()
        }

        function addTab(name, backgroundColor, profile) {
            let item = viewStack.itemAt(count - 1)
            const counter = item ? item.unreadPosts : 0

            let component = guiSettings.createComponent("SkyTabWithCloseButton.qml")

            // Creates with null parent, otherwise the button will be added
            // immediately to the end of that tab bar
            let tab = component.createObject(null, {
                    text: name,
                    backgroundColor: backgroundColor,
                    profile: profile,
                    counter: counter})
            tab.onPressAndHold.connect(() => showTimelineViewsSorter())
            tab.onClosed.connect(() => page.closeView(tab))
            console.debug("Add tab:", count - 1, name)
            insertItem(count - 1, tab) // Last item is the settings tab button
            setCurrentIndex(count - 2)
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

    BusyIndicator {
        anchors.centerIn: parent
        running: !parent.enabled
    }

    function getHeaderHeight() {
        if (currentViewItem && typeof currentViewItem.getHeaderHeight == 'function')
            return currentViewItem.getHeaderHeight()

        return 0
    }

    function resetHeaderPosition() {
        if (currentViewItem)
            currentViewItem.resetHeaderPosition()
    }

    function setInSync(index, offsetY = 0) {
        timelineView.setInSync(index, offsetY)
    }

    function stopSync() {
        timelineView.stopSync()
    }

    function moveToHome() {
        if (currentViewItem)
            currentViewItem.moveToHome()
    }

    function moveToPost(index) {
        timelineView.moveToPost(index)
    }

    function resumeTimeline(index, offsetY) {
        timelineView.resumeTimeline(index, offsetY)
    }

    function resyncTimeline(index, offsetY) {
        timelineView.resyncTimeline(index, offsetY)
    }

    function showTimelineViewsSorter() {
        if (skywalker.timelineModel.filteredPostFeedModels.length < 2)
            return

        let component = guiSettings.createComponent("TimelineViewsSorter.qml")
        let sorter = component.createObject(page, { timelineModel: skywalker.timelineModel })
        sorter.onClosed.connect(() => { root.popStack() })
        root.pushStack(sorter)
    }

    function addUserView() {
        let component = guiSettings.createComponent("AddUserTimelineView.qml")
        let addViewPage = component.createObject(page, { skywalker: skywalker })
        addViewPage.onSelected.connect((profile) => { // qmllint disable missing-property
                page.showUserView(profile)
                root.popStack()
        })
        addViewPage.onClosed.connect(() => { root.popStack() }) // qmllint disable missing-property
        root.pushStack(addViewPage)
    }

    function addHashtagView() {
        let component = guiSettings.createComponent("AddHashtagTimelineView.qml")
        let addViewPage = component.createObject(page, { skywalker: skywalker })
        addViewPage.onSelected.connect((hashtag) => { // qmllint disable missing-property
                page.showHashtagView(hashtag)
                root.popStack()
        })
        addViewPage.onClosed.connect(() => { root.popStack() }) // qmllint disable missing-property
        root.pushStack(addViewPage)
    }

    function addFocusHashtagView() {
        let component = guiSettings.createComponent("AddFocusHashtagTimelineView.qml")
        let addViewPage = component.createObject(page, { skywalker: skywalker })
        addViewPage.onSelected.connect((focusHashtagEntry) => { // qmllint disable missing-property
                page.showFocusHashtagView(focusHashtagEntry)
                root.popStack()
        })
        addViewPage.onClosed.connect(() => { root.popStack() }) // qmllint disable missing-property
        root.pushStack(addViewPage)
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
