import QtQuick
import QtQuick.Controls

RadioButton {
    id: radio
    padding: 0

    indicator: Rectangle { width: 0 }

    contentItem: Label {
        padding: 5
        background: Rectangle {
            color: radio.checked ? guiSettings.buttonColor : "transparent"
            border.width: 1
            border.color: guiSettings.buttonColor
        }
        color: checked ? guiSettings.buttonTextColor : guiSettings.buttonColor
        text: radio.text
    }

    Accessible.role: Accessible.RadioButton
    Accessible.name: text
    Accessible.onToggleAction: toggle()

    GuiSettings {
        id: guiSettings
    }
}
