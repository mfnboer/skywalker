import QtQuick

SequentialAnimation {
    required property var target
    property int duration: 1000

    id: blinking
    loops: Animation.Infinite
    NumberAnimation { target: blinking.target; property: "opacity"; to: 0; duration: blinking.duration }
    NumberAnimation { target: blinking.target; property: "opacity"; to: 1; duration: blinking.duration }

    onStopped: target.opacity = 1
}
