import QtQuick
import QtQuick.Controls

SplitView {
    id: splitView

    handle: Rectangle {
        id: handleItem
        implicitWidth: SplitHandle.pressed ? 2 : 1
        color: SplitHandle.pressed ? guiSettings.separatorHighLightColor : guiSettings.separatorColor

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
        }
    }
}
