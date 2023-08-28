import QtQuick
import Qt5Compat.GraphicalEffects

Item {
    required property QtObject objectToRound
    property int radius: 10

    id: frame

    Rectangle {
        id: mask
        width: objectToRound.width
        height: objectToRound.height
        radius: frame.radius
        visible: false
    }
    OpacityMask {
        anchors.fill: objectToRound
        source: objectToRound
        maskSource: mask
    }

    Component.onCompleted: objectToRound.visible = false
}
