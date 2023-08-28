import QtQuick
import Qt5Compat.GraphicalEffects

Rectangle {
    required property QtObject objectToRound

    id: frame
    color: "transparent"
    radius: 10

    Rectangle {
        id: mask
        width: objectToRound.width
        height: objectToRound.height
        radius: frame.radius
        visible: false
        z: -1
    }
    OpacityMask {
        id: content
        anchors.fill: objectToRound
        source: objectToRound
        maskSource: mask
        z: -1
    }

    Component.onCompleted: { objectToRound.visible = false; objectToRound.z = -1 }
}
