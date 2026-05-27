import QtQuick
import QtQuick.Effects

Item {
    required property var objectToRound
    property int radius: guiSettings.radius

    id: frame

    Rectangle {
        id: mask
        anchors.fill: parent
        radius: frame.radius
        visible: false
        layer.enabled: true
    }

    Loader {
        anchors.fill: parent
        active: objectToRound !== null

        sourceComponent: MultiEffect {
            source: objectToRound
            anchors.fill: parent
            maskEnabled: true
            maskSource: mask
        }
    }
}
