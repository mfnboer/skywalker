import QtQuick
import skywalker

Rectangle {
    required property var modelData
    property int visualIndex: DelegateModel.itemsIndex
    property Skywalker skywalker: root.getSkywalker()

    signal clicked
    signal dragged

    id: rect
    color: guiSettings.backgroundColor

    Drag.active: mouseArea.drag.active
    Drag.source: rect
    Drag.hotSpot.y: height / 2

    DropArea {
        anchors.fill: parent

        onEntered: (drag) => {
                       console.debug("Entered:", parent.modelData, "src:", drag.source.modelData)
                       drag.source.setDragStartingPoint(parent)
                       visualModel.items.move(drag.source.visualIndex, parent.visualIndex)
                   }
    }

    function setDragStartingPoint(item) {
        mouseArea.start = Qt.point(item.x, item.y)
        mouseArea.startZ = item.z
    }

    function restoreDragStartingPoint() {
        x = mouseArea.start.x
        y = mouseArea.start.y
        z = mouseArea.startZ
    }

    MouseArea {
        property point start
        property int startZ

        id: mouseArea
        anchors.fill: parent

        drag.axis: Drag.YAxis

        onPressAndHold: (mouse) => {
                            parent.setDragStartingPoint(parent)
                            parent.z = z + listView.count
                            parent.color = guiSettings.dragHighLightColor
                            drag.target = rect
                            mouse.accepted = false
                        }

        onReleased: {
            if (drag.target) {
                parent.restoreDragStartingPoint()
                parent.color = guiSettings.backgroundColor
                drag.target = undefined
                rect.dragged()
            }
        }

        onClicked: rect.clicked()
    }
}
