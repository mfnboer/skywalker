import QtQuick

Image {
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

        if (retryCount >= 3)
            return

        let retrySeconds = 2 ** retryCount
        retryTimer.interval = retrySeconds * 1000
        retryTimer.start()
        retryCount++

        console.debug("Retry loading retry:", retryCount, "source:", img.source)
    }
}
