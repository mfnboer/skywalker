import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

ListView {
    required property var skywalker
    property bool inSync: false
    property int margin: 8
    property int unreadPosts: 0
    property int topIndexBeforeInsert: -1

    id: timelineView
    spacing: 0
    model: skywalker.timelineModel
    clip: true
    flickDeceleration: guiSettings.flickDeceleration
    ScrollIndicator.vertical: ScrollIndicator {}

    header: Rectangle {
        width: parent.width
        height: guiSettings.headerHeight
        z: guiSettings.headerZLevel
        color: guiSettings.headerColor

        RowLayout {
            id: headerRow
            width: parent.width
            height: guiSettings.headerHeight

            Text {
                id: headerTexts
                Layout.alignment: Qt.AlignVCenter
                leftPadding: 10
                font.bold: true
                font.pointSize: guiSettings.scaledFont(10/8)
                color: guiSettings.headerTextColor
                text: qsTr("Home feed")
            }
            Item {
                Layout.rightMargin: 10
                Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                height: parent.height - 10
                width: height

                Avatar {
                    id: avatar
                    width: parent.width
                    height: parent.height
                    avatarUrl: skywalker.avatarUrl
                    onClicked: root.showSettingsDrawer()
                    onPressAndHold: root.showSwitchUserDrawer()
                }
            }
        }
    }
    headerPositioning: ListView.OverlayHeader

    footer: SkyFooter {
        timeline: timelineView
        skywalker: timelineView.skywalker
        homeActive: true
        onHomeClicked: moveToPost(0)
        onNotificationsClicked: root.viewNotifications()
        onSearchClicked: root.viewSearchView()
    }
    footerPositioning: ListView.OverlayFooter

    delegate: PostFeedViewDelegate {
        viewWidth: timelineView.width
    }

    onCountChanged: {
        if (!inSync)
            return

        let firstVisibleIndex = getFirstVisibleIndex()
        let lastVisibleIndex = getLastVisibleIndex()
        console.debug("COUNT CHANGED First:", firstVisibleIndex, "Last:", lastVisibleIndex, "Count:", count)
        // Adding/removing content changes the indices.
        skywalker.timelineMovementEnded(firstVisibleIndex, lastVisibleIndex)
        updateUnreadPosts(firstVisibleIndex)
    }

    onMovementEnded: {
        if (!inSync)
            return

        let firstVisibleIndex = getFirstVisibleIndex()
        let lastVisibleIndex = getLastVisibleIndex()
        console.info("END MOVEMENT First:", firstVisibleIndex, "Last:", lastVisibleIndex, "Count:", count, "AtBegin:", atYBeginning)
        skywalker.timelineMovementEnded(firstVisibleIndex, lastVisibleIndex)
        updateUnreadPosts(firstVisibleIndex)
    }

    FlickableRefresher {
        inProgress: skywalker.getTimelineInProgress
        verticalOvershoot: timelineView.verticalOvershoot
        topOvershootFun: () => skywalker.getTimeline(50)
        bottomOvershootFun: () => skywalker.getTimelineNextPage()
        enabled: timelineView.inSync
        topText: qsTr("Pull down to refresh timeline")
    }

    SvgImage {
        id: noPostImage
        width: 150
        height: 150
        y: height + (parent.headerItem ? parent.headerItem.height : 0)
        anchors.horizontalCenter: parent.horizontalCenter
        color: Material.color(Material.Grey)
        svg: svgOutline.noPosts
        visible: timelineView.count === 0
    }
    Text {
        id: noPostText
        y: noPostImage.y
        anchors.horizontalCenter: parent.horizontalCenter
        font.pointSize: guiSettings.scaledFont(10/8)
        color: Material.color(Material.Grey)
        elide: Text.ElideRight
        text: qsTr("No posts, follow more people")
        visible: timelineView.count === 0
    }

    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        running: skywalker.getTimelineInProgress && !skywalker.autoUpdateTimelineInProgress
    }

    GuiSettings {
        id: guiSettings
    }

    function getFirstVisibleIndex() {
        let firstVisibleIndex = indexAt(0, contentY)

        if (firstVisibleIndex < 0 && count > 0)
            return 0

        return firstVisibleIndex
    }

    function getLastVisibleIndex() {
        let lastVisibleIndex = indexAt(0, contentY + height - 1)

        if (lastVisibleIndex < 0 && count > 0)
            return count

        return lastVisibleIndex
    }

    function updateUnreadPosts(firstIndex) {
        timelineView.unreadPosts = Math.max(firstIndex, 0)
    }

    function moveToPost(index) {
        positionViewAtIndex(index, ListView.Beginning)
        updateUnreadPosts(index)
    }

    function rowsInsertedHandler(parent, start, end) {
        console.debug("ROWS INSERTED:", start, end, "Count:", count, "First visible:", getFirstVisibleIndex())

        if (topIndexBeforeInsert === 0 && start === 0) {
            if (count > end + 1) {
                // Stay at the current item instead of scrolling to the new top
                console.debug("Position at:", end + 1)
                positionViewAtIndex(end + 1, ListView.Beginning)
            }
            else {
                // TODO: remove code?
                // Avoid the flick to bounce down the timeline
                console.debug("Avoid flick WHY")
                // cancelFlick()
                // positionViewAtBeginning()
            }
        }

        const firstVisibleIndex = getFirstVisibleIndex()
        updateUnreadPosts(firstVisibleIndex)
    }

    function rowsAboutToBeInsertedHandler(parent, start, end) {
        topIndexBeforeInsert = getFirstVisibleIndex()
        console.debug("Top index before insert:", topIndexBeforeInsert, "Count:", count)
    }

    function setInSync(index) {
        moveToPost(index)
        inSync = true
        model.onRowsInserted.connect(rowsInsertedHandler)
        model.onRowsAboutToBeInserted.connect(rowsAboutToBeInsertedHandler)
    }

    function stopSync() {
        inSync = false
        model.onRowsInserted.disconnect(rowsInsertedHandler)
        model.onRowsAboutToBeInserted.disconnect(rowsAboutToBeInsertedHandler)
    }
}
