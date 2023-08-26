import QtQuick
import QtQuick.Layouts
import Qt5Compat.GraphicalEffects

Rectangle {
    property string source
    property int fillMode

    Layout.fillWidth: true
    height: fillMode === Image.PreserveAspectFit ? img.height : undefined

    Image {
        id: img
        width: parent.width
        height: parent.fillMode === Image.PreserveAspectCrop ? parent.height : undefined
        Layout.fillWidth: true
        fillMode: parent.fillMode
        source: parent.source
        visible: false
    }
    Rectangle {
        id: imgMask
        width: img.width
        height: img.height
        radius: 10
        visible: false
    }
    OpacityMask {
        anchors.fill: img
        source: img
        maskSource: imgMask
    }
}
