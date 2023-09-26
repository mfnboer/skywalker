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
    property bool gettingNewPosts: false
    property bool inBottomOvershoot: false

    id: timelineView
    spacing: 0
    model: skywalker.timelineModel
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
                leftPadding: 8
                font.bold: true
                font.pointSize: guiSettings.scaledFont(10/8)
                color: "white"
                text: qsTr("Following timeline")
            }
            Item {
                Layout.rightMargin: 8
                Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                height: parent.height - 16
                width: height

                Avatar {
                    id: avatar
                    width: parent.width
                    height: parent.height
                    avatarUrl: skywalker.avatarUrl
                }
            }
        }
    }
    headerPositioning: ListView.OverlayHeader

    footer: Rectangle {
        id: viewFooter
        width: parent.width
        height: guiSettings.footerHeight
        z: guiSettings.footerZLevel
        color: guiSettings.footerColor

        Row {
            width: parent.width

            SvgImage {
                id: homeButton
                y: height + 5
                width: height
                height: guiSettings.footerHeight - 10
                Layout.fillWidth: true
                color: guiSettings.textColor
                svg: svgOutline.home

                Rectangle {
                    x: parent.width - 17
                    y: -parent.y + 6
                    width: Math.max(unreadCountText.width + 10, height)
                    height: 20
                    radius: 8
                    color: guiSettings.badgeColor
                    border.color: guiSettings.badgeBorderColor
                    border.width: 2
                    visible: timelineView.unreadPosts > 0

                    Text {
                        id: unreadCountText
                        anchors.centerIn: parent
                        font.bold: true
                        font.pointSize: guiSettings.scaledFont(6/8)
                        color: guiSettings.badgeTextColor
                        text: timelineView.unreadPosts
                    }
                }

                MouseArea {
                    y: -parent.y
                    width: parent.width
                    height: parent.height
                    onClicked: moveToPost(0)
                }
            }

            SvgImage {
                id: notificationsButton
                y: height + 5
                width: height
                height: guiSettings.footerHeight - 10
                Layout.fillWidth: true
                color: guiSettings.textColor
                svg: svgOutline.notifications

                Rectangle {
                    x: parent.width - 17
                    y: -parent.y + 6
                    width: Math.max(unreadNotificationsText.width + 10, height)
                    height: 20
                    radius: 8
                    color: guiSettings.badgeColor
                    border.color: guiSettings.badgeBorderColor
                    border.width: 2
                    visible: false // TODO

                    Text {
                        id: unreadNotificationsText
                        anchors.centerIn: parent
                        font.bold: true
                        font.pointSize: guiSettings.scaledFont(6/8)
                        color: guiSettings.badgeTextColor
                        text: "0" // TODO
                    }
                }

                MouseArea {
                    y: -parent.y
                    width: parent.width
                    height: parent.height
                    onClicked: root.viewNotifications()
                }
            }
        }

        SvgButton {
            x: parent.width - width - 10
            y: -height - 10
            width: 70
            height: width
            iconColor: guiSettings.buttonTextColor
            Material.background: guiSettings.buttonColor
            opacity: 0.6
            imageMargin: 20
            svg: svgOutline.chat
            onClicked: root.composePost()
        }
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

        if (verticalOvershoot < 0)  {
            if (!inTopOvershoot && !skywalker.getTimelineInProgress) {
                gettingNewPosts = true
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

    BusyIndicator {
        id: busyTopIndicator
        y: parent.y + guiSettings.headerHeight
        anchors.horizontalCenter: parent.horizontalCenter
        running: gettingNewPosts
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

    function setInSync(index) {
        moveToPost(index)
        inSync = true

        model.onRowsAboutToBeInserted.connect((parent, start, end) => {
                console.debug("ROWS TO INSERT:", start, end, "AtBegin:", atYBeginning)
            })

        model.onRowsInserted.connect((parent, start, end) => {
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
            })
    }

    Component.onCompleted: {
        skywalker.onGetTimeLineInProgressChanged.connect(() => {
                if (!skywalker.getTimelineInProgress)
                    gettingNewPosts = false
            })
    }
}
