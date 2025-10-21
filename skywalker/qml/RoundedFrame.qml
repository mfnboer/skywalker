import QtQuick
import Qt5Compat.GraphicalEffects

Rectangle {
    required property QtObject objectToRound

    id: frame
    color: "transparent"
    radius: guiSettings.radius
    height: objectToRound.height

    Rectangle {
        id: mask
        width: objectToRound.width
        height: objectToRound.height
        radius: frame.radius
        visible: false
    }
    OpacityMask {
        id: content
        anchors.fill: objectToRound
        source: objectToRound
        maskSource: mask
    }

    Component.onCompleted: {
        objectToRound.visible = false
    }
}
