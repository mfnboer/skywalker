import QtQuick
import QtQuick.Controls

Page {
    // Called when list gets covered by another page
    signal cover

    Material.background: guiSettings.backgroundColor

    GuiSettings {
        id: guiSettings
    }
}
