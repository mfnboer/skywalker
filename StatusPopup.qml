import QtQuick
import QtQuick.Controls
import skywalker

Popup {
    property int level // QEnums::StatusLevel

    id: statusPopup
    width: parent.width
    background: Rectangle {
        color: Material.color([Material.Green, Material.Red][level])
    }
    closePolicy: Popup.CloseOnPressOutside

    Label {
        id: statusText
        width: parent.width
        wrapMode: Text.Wrap
        maximumLineCount: 2
        elide: Text.ElideRight
        text: "Status"
    }

    Timer {
        id: closeTimer
        interval: 5000
        onTriggered: statusPopup.close()
    }

    function show(msg, level = QEnums.STATUS_LEVEL_INFO) {
        statusPopup.level = level
        statusText.text = msg;
        open()
        closeTimer.restart()
    }
}
