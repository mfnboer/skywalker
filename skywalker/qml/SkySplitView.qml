import QtQuick
import QtQuick.Controls

SplitView {
    id: splitView

    handle: Rectangle {
        id: handleItem
        implicitWidth: 2
        color: SplitHandle.pressed ? guiSettings.separatorHighLightColor : guiSettings.backgroundColor

        containmentMask: Item {
            x: (handleItem.width - width) / 2
            width: 30
            height: rootSplitView.height
        }

        Rectangle {
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            width: 1
            height: 15
            color: guiSettings.separatorHighLightColor
        }

        Rectangle {
            anchors.right: parent.right
            anchors.rightMargin: 4
            anchors.verticalCenter: parent.verticalCenter
            width: 1
            height: 15
            color: guiSettings.separatorHighLightColor
        }
    }
}
