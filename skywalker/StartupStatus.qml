import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyPage {
    property bool isRewinding: false
    property int rewindMaxPages
    property int rewindPagesLeft
    property date rewindTimestamp
    property date rewindProgressTimestamp
    property int rewindTotalMs
    property double rewindProgress: 0.0

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
            padding: 10
            color: "white"
            font.bold: true
            font.pointSize: guiSettings.scaledFont(3.5)
            text: skywalker.APP_NAME
        }
        AccessibleText {
            id: status
            anchors.horizontalCenter: parent.horizontalCenter
            padding: 10
            font.pointSize: guiSettings.scaledFont(2)

            SequentialAnimation on color {
                loops: Animation.Infinite
                ColorAnimation { from: "white"; to: guiSettings.skywalkerLogoColor; duration: 1000 }
                ColorAnimation { from: guiSettings.skywalkerLogoColor; to: "white"; duration: 1000 }
            }
        }
        ProgressBar {
            id: rewindProgressBar
            x: 10
            width: parent.width - 20
            from: 0.0
            to: 1.0
            value: rewindProgress
            visible: isRewinding

            contentItem: Item {
                implicitWidth: parent.width
                implicitHeight: 4

                Rectangle {
                    width: rewindProgressBar.visualPosition * parent.width
                    height: parent.height
                    color: "white"
                }
            }
        }
        AccessibleText {
            width: parent.width
            rightPadding: 10
            horizontalAlignment: Qt.AlignRight
            text: `${(Math.round(rewindProgressBar.value * 100))}%`
            font.pointSize: guiSettings.scaledFont(7/8)
            color: "white"
            visible: isRewinding

            AccessibleText {
                anchors.horizontalCenter: parent.horizontalCenter
                text: rewindProgressTimestamp.toLocaleString(Qt.locale(), Locale.ShortFormat)
                font.pointSize: guiSettings.scaledFont(7/8)
                color: "white"
            }
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

    function setRewindProgress() {
        const pageProgress = (rewindMaxPages - rewindPagesLeft) / rewindMaxPages
        const msLeft = rewindProgressTimestamp - rewindTimestamp
        const timeProgress = (rewindTotalMs - msLeft) / rewindTotalMs
        rewindProgress = Math.max(pageProgress, timeProgress)
        console.debug("Rewind progress maxPages:", rewindMaxPages, "pagesLeft:", rewindPagesLeft, "pageProgress:", pageProgress)
        console.debug("Rewind progress totalMs:", rewindTotalMs, "msLeft:", msLeft, "timeProgress:", timeProgress)
    }

    function startRewind(maxPages, timestamp) {
        rewindMaxPages = maxPages
        rewindPagesLeft = maxPages
        rewindTimestamp = timestamp
        rewindProgressTimestamp = new Date()
        rewindTotalMs = rewindProgressTimestamp - rewindTimestamp
        isRewinding = true
        setRewindProgress()
    }

    function updateRewindProgress(pages, timestamp) {
        rewindPagesLeft = pages
        rewindProgressTimestamp = timestamp
        setRewindProgress()
    }
}
