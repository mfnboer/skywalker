import QtQuick
import QtQuick.Controls

RadioButton {
    id: radio
    padding: 0

    indicator: Rectangle { width: 0 }

    contentItem: Label {
        padding: 5
        background: Rectangle {
            color: radio.checked ? "blue" : "transparent"
            border.width: 1
            border.color: "blue"
        }
        color: checked ? "white" : "blue"
        text: radio.text
    }

    GuiSettings {
        id: guiSettings
    }
}
