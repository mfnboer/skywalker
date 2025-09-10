import QtQuick
import QtQuick.Controls

Column {
    property string color: guiSettings.textColor
    property bool isRewinding: false
    property int rewindMaxPages
    property int rewindPagesLeft
    property date rewindTimestamp
    property date rewindProgressTimestamp
    property int rewindTotalMs
    property double rewindProgress: 0.0
    readonly property int rewindPagesLoaded: rewindMaxPages - rewindPagesLeft

    id: rewindStatus
    visible: isRewinding

    ProgressBar {
        id: rewindProgressBar
        width: parent.width
        from: 0.0
        to: 1.0
        value: rewindProgress


        contentItem: Item {
            implicitWidth: parent.width
            implicitHeight: 4

            Rectangle {
                width: rewindProgressBar.visualPosition * parent.width
                height: parent.height
                color: rewindStatus.color
            }
        }
    }
    AccessibleText {
        width: parent.width
        horizontalAlignment: Qt.AlignRight
        text: `${(Math.round(rewindProgressBar.value * 100))}%`
        font.pointSize: guiSettings.scaledFont(7/8)
        color: rewindStatus.color

        AccessibleText {
            anchors.horizontalCenter: parent.horizontalCenter
            text: rewindProgressTimestamp.toLocaleString(Qt.locale(), Locale.ShortFormat)
            font.pointSize: guiSettings.scaledFont(7/8)
            color: rewindStatus.color
        }
    }

    function setRewindProgress() {
        const pageProgress = rewindPagesLoaded / rewindMaxPages
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
