import QtQuick

Image {
    property int maxRetry: 5
    property int retryCount: 0

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

        let retryMs = 2 ** Math.min(retryCount, 5)
        retryTimer.interval = retryMs * 1000
        retryTimer.start()
        retryCount++

        console.debug("Retry loading retry:", retryCount, "source:", img.source)
    }
}
