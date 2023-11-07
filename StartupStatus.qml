import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Page {
    signal closed()

    width: parent.width
    height: parent.height
    background: Rectangle { color: guiSettings.skywalkerLogoColor }

    Column {
        width: parent.width
        height: parent.height

        Text {
            id: title
            anchors.horizontalCenter: parent.horizontalCenter
            padding: 10
            color: "white"
            font.bold: true
            font.pointSize: guiSettings.scaledFont(3.5)
            text: "Skywalker"
        }
        Text {
            id: status
            anchors.horizontalCenter: parent.horizontalCenter
            padding: 10
            color: "white"
            font.pointSize: guiSettings.scaledFont(2)

            SequentialAnimation on color {
                loops: Animation.Infinite
                ColorAnimation { from: "white"; to: guiSettings.skywalkerLogoColor; duration: 1000 }
                ColorAnimation { from: guiSettings.skywalkerLogoColor; to: "white"; duration: 1000 }
            }
        }
        Image {
            anchors.horizontalCenter: parent.horizontalCenter
            width: height
            height: Math.min(parent.height - title.height - status.height, parent.width)
            fillMode: Image.PreserveAspectFit
            source: "images/skywalker.png"
        }
    }

    GuiSettings {
        id: guiSettings
    }

    function setStatus(msg) {
        status.text = msg
    }
}
