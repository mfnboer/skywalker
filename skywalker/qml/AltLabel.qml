import QtQuick
import QtQuick.Controls

Label {
    property string backgroundColor: "black"

    anchors.left: parent.left
    anchors.bottom: parent.bottom
    leftInset: 4
    rightInset: 4
    topInset: 5
    bottomInset: 5
    padding: 5
    color: "white"
    font.pointSize: guiSettings.scaledFont(4.5/8)
    text: qsTr("ALT", "alternative text indication on an image")

    background: Rectangle {
        radius: 3
        color: backgroundColor
        opacity: 0.6
    }
}
