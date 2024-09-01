import QtQuick
import QtQuick.Controls

ListView {
    property bool enableOnScreenCheck: false

    spacing: 0
    clip: true
    flickDeceleration: guiSettings.flickDeceleration
    maximumFlickVelocity: guiSettings.maxFlickVelocity
    pixelAligned: guiSettings.flickPixelAligned
    cacheBuffer: 10000
    ScrollIndicator.vertical: ScrollIndicator {}

    Accessible.role: Accessible.List

    onMovementEnded: {
        if (!enableOnScreenCheck)
            return

        for (var i = 0; i < count; ++i) {
            const item = itemAtIndex(i)

            if (item)
                item.checkOnScreen()
        }
    }

    GuiSettings {
        id: guiSettings
    }
}
