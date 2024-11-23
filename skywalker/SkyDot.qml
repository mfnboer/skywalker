import QtQuick

Rectangle {
    anchors.top: parent.top
    anchors.right: parent.right
    anchors.topMargin: 5
    anchors.rightMargin: 5
    width: 10
    height: width
    radius: width / 2
    color: guiSettings.accentColor
}
