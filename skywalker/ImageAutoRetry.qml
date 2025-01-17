import QtQuick
import QtQuick.Controls
import skywalker

Image {
    property int maxRetry: 5
    property int retryCount: 0
    property bool indicateLoading: true
    property string reloadIconColor: guiSettings.textColor
    readonly property bool failedCanReload: status == Image.Error && retryCount >= maxRetry && indicateLoading

    id: img

    Timer {
        id: retryTimer

        onTriggered: {
            let src = img.source
            img.source = ""
            img.source = src
        }
    }

    onStatusChanged: {
        if (status != Image.Error)
            return

        console.debug("Failed to load image:", img.source)

        if (retryCount >= maxRetry)
            return

        let retryMs = 2 ** Math.min(retryCount, 4)
        retryTimer.interval = retryMs * 1000
        retryTimer.start()
        retryCount++

        console.debug("Retry loading retry:", retryCount, "source:", img.source)
    }

    BusyIndicator {
        width: 30
        height: width
        anchors.centerIn: parent
        running: parent.status == Image.Loading && indicateLoading
    }

    SvgButton {
        id: reloadButton
        anchors.centerIn: parent
        iconColor: reloadIconColor
        Material.background: "transparent"
        svg: SvgOutline.refresh
        accessibleName: qsTr("reload")
        visible: failedCanReload
        onClicked: reload()
    }
    AccessibleText {
        anchors.top: reloadButton.bottom
        anchors.horizontalCenter: reloadButton.horizontalCenter
        color: Material.color(Material.Grey)
        text: qsTr("Loading failed")
        visible: failedCanReload
    }

    function reload() {
        retryCount = 0
        retryTimer.interval = 1
        retryTimer.start()
    }
}
