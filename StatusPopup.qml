import QtQuick
import QtQuick.Controls
import skywalker

Popup {
    property int level // QEnums::StatusLevel

    id: statusPopup
    width: parent.width
    height: 60
    z: 200

    background: Rectangle {
        color: ["lightcyan", "crimson"][level]
        border.width: 1
        border.color: Material.color(Material.Grey)
        radius: 5
    }
    closePolicy: Popup.CloseOnPressOutside

    Label {
        id: statusText
        anchors.left: parent.left
        anchors.right: closeButton.left
        anchors.verticalCenter: parent.verticalCenter
        wrapMode: Text.Wrap
        maximumLineCount: 4
        elide: Text.ElideRight
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

    function show(msg, level = QEnums.STATUS_LEVEL_INFO, intervalSec = 30) {
        console.debug("Level:", level, "Msg:", msg);
        statusPopup.level = level
        statusText.text = msg;
        open()
        closeTimer.interval = intervalSec * 1000
        closeTimer.restart()
    }
}
