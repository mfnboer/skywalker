import QtQuick
import QtQuick.Controls

SplitView {
    property bool locked: false

    id: splitView

    handle: Rectangle {
        id: handleItem
        implicitWidth: (SplitHandle.pressed && !locked) ? 2 : 1
        color: (SplitHandle.pressed && !locked) ? guiSettings.separatorHighLightColor : guiSettings.separatorColor
        enabled: !locked

        containmentMask: Item {
            x: (handleItem.width - width) / 2
            width: 30
            height: rootSplitView.height
        }

        Rectangle {
            anchors.right: parent.right
            anchors.rightMargin: 4
            anchors.verticalCenter: parent.verticalCenter
            width: 2
            height: 20
            color: guiSettings.separatorHighLightColor
            visible: !locked
        }
    }
}
