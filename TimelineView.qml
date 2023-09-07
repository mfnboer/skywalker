import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

ListView {
    property int margin: 8

    property bool inTopOvershoot: false
    property bool inBottomOvershoot: false
    property string avatarUrl: ""

    id: timelineView
    spacing: 0
    model: skywalker.timelineModel
    ScrollIndicator.vertical: ScrollIndicator {}

    header: Rectangle {
        width: parent.width
        height: 44
        z: 10
        color: "black"

        RowLayout {
            id: headerRow
            width: parent.width
            height: 44

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
                    avatarUrl: timelineView.avatarUrl
                }
            }
        }
    }
    headerPositioning: ListView.OverlayHeader

    delegate: PostFeedViewDelegate {
        viewWidth: timelineView.width
    }

    onCountChanged: {
        let firstVisibleIndex = indexAt(0, contentY)
        let lastVisibleIndex = indexAt(0, contentY + height - 1)
        console.debug("COUNT CHANGED First:", firstVisibleIndex, "Last:", lastVisibleIndex, "Count:", count)
        // Adding/removing content changes the indices.
        skywalker.timelineMovementEnded(firstVisibleIndex, lastVisibleIndex)
    }

    onMovementEnded: {
        let firstVisibleIndex = indexAt(0, contentY)
        let lastVisibleIndex = indexAt(0, contentY + height - 1)
        console.debug("END MOVEMENT First:", firstVisibleIndex, "Last:", lastVisibleIndex, "Count:", count)
        skywalker.timelineMovementEnded(firstVisibleIndex, lastVisibleIndex)
    }

    onVerticalOvershootChanged: {
        if (verticalOvershoot < 0)  {
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
}
