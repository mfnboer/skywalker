import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

ListView {
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
        height: root.headerHeight
        z: 10
        color: "black"

        RowLayout {
            id: headerRow
            width: parent.width
            height: root.headerHeight

            Text {
                id: headerTexts
                Layout.alignment: Qt.AlignVCenter
                leftPadding: 8
                font.bold: true
                font.pointSize: root.scaledFont(10/8)
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
        height: root.footerHeight
        z: 10
        color: "white"

        Row {
            width: parent.width

            SvgImage {
                y: height + 5
                width: 34
                height: 34
                id: homeButton
                color: "black"
                svg: svgOutline.home

                Rectangle {
                    x: parent.width - 14
                    y: -parent.y + 5
                    width: Math.max(unreadCountText.width + 10, 18)
                    height: 18
                    radius: 8
                    color: "blue"
                    border.color: "white"
                    border.width: 2
                    visible: timelineView.unreadPosts > 0

                    Text {
                        id: unreadCountText
                        anchors.centerIn: parent
                        font.bold: true
                        font.pointSize: root.scaledFont(6/8)
                        color: "white"
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
        }

        SvgButton {
            x: parent.width - width - 10
            y: -height - 10
            width: 70
            height: width
            iconColor: "white"
            Material.background: "blue"
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
        let firstVisibleIndex = indexAt(0, contentY)
        let lastVisibleIndex = indexAt(0, contentY + height - 1)
        console.debug("COUNT CHANGED First:", firstVisibleIndex, "Last:", lastVisibleIndex, "Count:", count)
        // Adding/removing content changes the indices.
        skywalker.timelineMovementEnded(firstVisibleIndex, lastVisibleIndex)
        updateUnreadPosts(firstVisibleIndex)
    }

    onMovementEnded: {
        let firstVisibleIndex = indexAt(0, contentY)
        let lastVisibleIndex = indexAt(0, contentY + height - 1)
        console.info("END MOVEMENT First:", firstVisibleIndex, "Last:", lastVisibleIndex, "Count:", count, "AtBegin:", atYBeginning)
        skywalker.timelineMovementEnded(firstVisibleIndex, lastVisibleIndex)
        updateUnreadPosts(firstVisibleIndex)
    }

    onVerticalOvershootChanged: {
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
        y: parent.y + root.headerHeight
        anchors.horizontalCenter: parent.horizontalCenter
        running: gettingNewPosts
    }

    function updateUnreadPosts(firstIndex) {
        timelineView.unreadPosts = Math.max(firstIndex, 0)
    }

    function moveToPost(index) {
        positionViewAtIndex(index, ListView.Beginning)
        updateUnreadPosts(index)
    }

    Component.onCompleted: {
        skywalker.onGetTimeLineInProgressChanged.connect(() => {
                if (!skywalker.getTimelineInProgress)
                    gettingNewPosts = false
            })

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
}
