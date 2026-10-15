import QtQuick
import QtQuick.Controls
import skywalker

Image {
    property int maxRetry: 5
    property int retryCount: 0
    property bool indicateLoading: true
    property string reloadIconColor: guiSettings.textColor
    property bool isResetting: false
    readonly property bool failedCanReload: status == Image.Error && retryCount >= maxRetry && indicateLoading
    readonly property bool isLoading: status == Image.Loading || (status == Image.Error && retryCount < maxRetry) || isResetting

    id: img
    asynchronous: true

    Timer {
        id: retryTimer

        onTriggered: {
            isResetting = true
            let src = img.source
            img.source = ""
            img.source = src
            isResetting = false
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
        running: isLoading && indicateLoading
    }

    Loader {
        width: parent.width
        anchors.centerIn: parent
        active: failedCanReload

        sourceComponent: Item {
            width: parent.width
            height: reloadButton.height + reloadText.height

            SvgButton {
                id: reloadButton
                anchors.horizontalCenter: parent.horizontalCenter
                iconColor: reloadIconColor
                Material.background: "transparent"
                svg: SvgOutline.refresh
                accessibleName: qsTr("reload")
                onClicked: reload()
            }
            AccessibleText {
                id: reloadText
                anchors.top: reloadButton.bottom
                anchors.horizontalCenter: parent.horizontalCenter
                color: Material.color(Material.Grey)
                text: qsTr("Loading failed")
            }
        }
    }

    function reload() {
        console.debug("Reload img:", source)
        retryCount = 0
        retryTimer.interval = 1
        retryTimer.start()
    }

    function getVisible() {
        return visible
    }

    function setVisible(v) {
        visible = v
    }
}
