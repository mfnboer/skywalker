import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import skywalker

Popup {
    property string userDid
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

    onOpened: statusText.forceActiveFocus()

    Loader {
        id: currentUserAvatar
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        height: active ? guiSettings.headerHeight - 10 : 0
        width: height
        active: !root.isActiveUser(userDid)

        sourceComponent: CurrentUserAvatar {
            userDid: statusPopup.userDid
        }
    }

    Label {
        id: statusText
        padding: 10
        anchors.left: currentUserAvatar.active ? currentUserAvatar.right : parent.left
        anchors.right: closeButton.left
        anchors.verticalCenter: parent.verticalCenter
        wrapMode: Text.Wrap
        maximumLineCount: 8
        elide: Text.ElideRight
        color: guiSettings.textColor
        text: "Status"

        Accessible.role: Accessible.StaticText
        Accessible.name: text
        Accessible.description: Accessible.name
    }
    SvgPlainButton {
        id: closeButton
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        iconColor: statusText.color.toString()
        svg: SvgOutline.close
        accessibleName: qsTr("close status message")
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

    function show(userDid, msg, level = QEnums.STATUS_LEVEL_INFO, intervalSec = 30) {
        console.debug("Level:", level, "Msg:", msg);
        statusPopup.userDid = userDid
        statusPopup.level = level
        statusText.text = msg;
        open()
        closeTimer.interval = intervalSec * 1000
        closeTimer.restart()
    }
}
