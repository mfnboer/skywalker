import QtQuick
import QtQuick.Controls

RadioButton {
    id: radio
    indicator: Rectangle {}

    contentItem: Label {
        padding: 5
        background: Rectangle { color: radio.checked ? "blue" : "transparent" }
        color: checked ? "white" : guiSettings.textColor
        text: radio.text
    }

    GuiSettings {
        id: guiSettings
    }
}
