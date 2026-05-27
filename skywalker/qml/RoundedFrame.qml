import QtQuick
import QtQuick.Effects

Item {
    required property QtObject objectToRound
    property int radius: guiSettings.radius

    id: frame

    Rectangle {
        id: mask
        width: objectToRound.width
        height: objectToRound.height
        radius: frame.radius
        visible: false
        layer.enabled: true
    }
    MultiEffect {
        source: objectToRound
        anchors.fill: objectToRound
        maskEnabled: true
        maskSource: mask
    }

    Component.onCompleted: {
        objectToRound.visible = false
    }
}
