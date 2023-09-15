import QtQuick
import QtQuick.Controls
import skywalker

Popup {
    property int level // QEnums::StatusLevel

    id: statusPopup
    width: parent.width
    height: 50

    background: Rectangle {
        color: ["lightcyan", "red"][level]
        border.width: 1
        border.color: "grey"
        radius: 5
    }
    closePolicy: Popup.CloseOnPressOutside

    Label {
        id: statusText
        anchors.left: parent.left
        anchors.right: closeButton.left
        anchors.verticalCenter: parent.verticalCenter
        wrapMode: Text.Wrap
        maximumLineCount: 2
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
        interval: 30000
        onTriggered: statusPopup.close()
    }

    function show(msg, level = QEnums.STATUS_LEVEL_INFO) {
        statusPopup.level = level
        statusText.text = msg;
        open()
        closeTimer.restart()
    }
}
