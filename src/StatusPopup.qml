import QtQuick
import QtQuick.Controls
import skywalker

Popup {
    property int level // QEnums::StatusLevel

    id: statusPopup
    width: parent.width
    height: Math.max(statusText.height, closeButton.height)
    z: 200

    background: Rectangle {
        color: [Material.theme === Material.Light ? "lightcyan" : "teal",
                Material.theme === Material.Light ? "crimson" : "darkred"][level]
        border.width: 1
        border.color: guiSettings.borderColor
        radius: 5
    }
    closePolicy: Popup.CloseOnPressOutside

    Label {
        id: statusText
        padding: 10
        anchors.left: parent.left
        anchors.right: closeButton.left
        anchors.verticalCenter: parent.verticalCenter
        wrapMode: Text.Wrap
        maximumLineCount: 8
        elide: Text.ElideRight
        color: guiSettings.textColor
        text: "Status"
    }
    SvgButton {
        id: closeButton
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        iconColor: statusText.color
        Material.background: "transparent"
        svg: svgOutline.close
        onClicked: statusPopup.close()
    }

    MouseArea {
        anchors.fill: statusText
        onClicked: statusPopup.close()
    }

    Timer {
        id: closeTimer
        onTriggered: statusPopup.close()
    }

    GuiSettings {
        id: guiSettings
    }

    function show(msg, level = QEnums.STATUS_LEVEL_INFO, intervalSec = 30) {
        console.debug("Level:", level, "Msg:", msg);
        statusPopup.level = level
        statusText.text = msg;
        open()
        closeTimer.interval = intervalSec * 1000
        closeTimer.restart()
    }
}