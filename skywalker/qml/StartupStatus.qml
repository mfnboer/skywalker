import QtQuick
import QtQuick.Controls
import skywalker

SkyPage {
    readonly property bool noSideBar: true

    signal closed()

    width: parent.width
    height: parent.height
    background: Rectangle { color: guiSettings.skywalkerLogoColor }

    Accessible.role: Accessible.Pane
    Accessible.name: qsTr("Signing in, please wait")

    Column {
        width: parent.width
        height: parent.height

        AccessibleText {
            id: title
            anchors.horizontalCenter: parent.horizontalCenter
            padding: 10 + guiSettings.headerMargin
            color: "white"
            font.bold: true
            font.pointSize: guiSettings.absScaledFont(3.5)
            text: skywalker.APP_NAME
        }

        AccessibleText {
            id: status
            anchors.horizontalCenter: parent.horizontalCenter
            padding: 10
            font.pointSize: guiSettings.absScaledFont(2)

            SequentialAnimation on color {
                loops: Animation.Infinite
                ColorAnimation { from: "white"; to: guiSettings.skywalkerLogoColor; duration: 1000 }
                ColorAnimation { from: guiSettings.skywalkerLogoColor; to: "white"; duration: 1000 }
            }
        }

        RewindStatus {
            id: rewindStatus
            x: 10
            width: parent.width - 20
            color: "white"
        }

        Image {
            anchors.horizontalCenter: parent.horizontalCenter
            width: height
            height: Math.min(parent.height - title.height - status.height, parent.width)
            fillMode: Image.PreserveAspectFit
            source: "/images/skywalker.png"

            Accessible.ignored: true
        }
    }

    function setStatus(msg) {
        status.text = msg
        status.forceActiveFocus()
    }

    function startRewind(maxPages, timestamp) {
        rewindStatus.startRewind(maxPages, timestamp)
    }

    function updateRewindProgress(pages, timestamp) {
        rewindStatus.updateRewindProgress(pages, timestamp)
    }

    Component.onDestruction: {
        displayUtils.setNavigationBarColor(guiSettings.backgroundColor)
        displayUtils.setStatusBarColor(guiSettings.headerColor)
    }

    Component.onCompleted: {
        displayUtils.setNavigationBarColorAndMode(guiSettings.skywalkerLogoColor, false)
        displayUtils.setStatusBarColorAndMode(guiSettings.skywalkerLogoColor, false)
    }
}
