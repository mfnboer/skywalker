import QtQuick

Rectangle {
    property string backgroundColor: guiSettings.backgroundColor

    anchors.fill: parent
    radius: guiSettings.radius
    color: guiSettings.highLightColor(backgroundColor)
    visible: parent.status !== Loader.Ready
}
