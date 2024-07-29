import QtQuick
import QtQuick.Controls

ListView {
    spacing: 0
    clip: true
    flickDeceleration: guiSettings.flickDeceleration
    maximumFlickVelocity: guiSettings.maxFlickVelocity
    pixelAligned: guiSettings.flickPixelAligned
    cacheBuffer: 10000
    ScrollIndicator.vertical: ScrollIndicator {}

    Accessible.role: Accessible.List

    GuiSettings {
        id: guiSettings
    }
}
