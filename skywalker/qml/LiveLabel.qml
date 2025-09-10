import QtQuick
import QtQuick.Controls

Label {
    padding: 1
    color: guiSettings.liveTextColor
    font.bold: true
    text: qsTr("LIVE")

    background: Rectangle {
        radius: 3
        color: guiSettings.liveColor
    }
}
