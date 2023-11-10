import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

ListView {
    required property var skywalker
    property bool inSync: false
    property int margin: 8
    property int unreadPosts: 0

    property bool inTopOvershoot: false
    property bool inBottomOvershoot: false

    id: timelineView
    spacing: 0
    model: skywalker.timelineModel
    clip: true
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

        let firstVisibleIndex = indexAt(0, contentY)
        let lastVisibleIndex = indexAt(0, contentY + height - 1)
        console.debug("COUNT CHANGED First:", firstVisibleIndex, "Last:", lastVisibleIndex, "Count:", count)
        // Adding/removing content changes the indices.
        skywalker.timelineMovementEnded(firstVisibleIndex, lastVisibleIndex)
        updateUnreadPosts(firstVisibleIndex)
    }

    onMovementEnded: {
        if (!inSync)
            return

        let firstVisibleIndex = indexAt(0, contentY)
        let lastVisibleIndex = indexAt(0, contentY + height - 1)
        console.info("END MOVEMENT First:", firstVisibleIndex, "Last:", lastVisibleIndex, "Count:", count, "AtBegin:", atYBeginning)
        skywalker.timelineMovementEnded(firstVisibleIndex, lastVisibleIndex)
        updateUnreadPosts(firstVisibleIndex)
    }

    onVerticalOvershootChanged: {
        if (!inSync)
            return

        console.debug("Vertical overshoot:", verticalOvershoot)
        if (verticalOvershoot < -refreshText.height - 2)  {
            if (!inTopOvershoot && !skywalker.getTimelineInProgress) {
                skywalker.getTimeline(50)
            }

            inTopOvershoot = true
        } else {
            inTopOvershoot = false
        }

        if (verticalOvershoot > 0) {
            if (!inBottomOvershoot && !skywalker.getTimelineInProgress) {
                skywalker.getTimelineNextPage()
            }

            inBottomOvershoot = true;
        } else {
            inBottomOvershoot = false;
        }
    }

    Text {
        id: refreshText
        anchors.horizontalCenter: parent.horizontalCenter
        y: guiSettings.headerHeight - verticalOvershoot - height
        z: parent.z - 1
        font.italic: true
        color: guiSettings.textColor
        text: qsTr("Refresh timeline");
        visible: verticalOvershoot < 0
    }

    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        running: skywalker.getTimelineInProgress && !skywalker.autoUpdateTimelineInProgress
    }

    GuiSettings {
        id: guiSettings
    }

    function updateUnreadPosts(firstIndex) {
        timelineView.unreadPosts = Math.max(firstIndex, 0)
    }

    function moveToPost(index) {
        positionViewAtIndex(index, ListView.Beginning)
        updateUnreadPosts(index)
    }

    function rowsInsertedHandler(parent, start, end) {
        console.debug("ROWS INSERTED:", start, end, "AtBegin:", atYBeginning)
        if (atYBeginning) {
            if (count > end + 1) {
                // Stay at the current item instead of scrolling to the new top
                positionViewAtIndex(end, ListView.Beginning)
            }
            else {
                // Avoid the flick to bounce down the timeline
                cancelFlick()
                positionViewAtBeginning()
            }
        }

        let firstVisibleIndex = indexAt(0, contentY)
        updateUnreadPosts(firstVisibleIndex)
    }

    function setInSync(index) {
        moveToPost(index)
        inSync = true
        model.onRowsInserted.connect(rowsInsertedHandler)
    }

    function stopSync() {
        inSync = false
        model.onRowsInserted.disconnect(rowsInsertedHandler)
    }
}
