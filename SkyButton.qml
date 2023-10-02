import QtQuick
import QtQuick.Controls

RoundButton {
    id: button

    Material.background: flat ? guiSettings.labelColor : guiSettings.buttonColor
    contentItem: Text {
        leftPadding: 10
        rightPadding: 10
        color: flat ? guiSettings.textColor : guiSettings.buttonTextColor
        text: button.text
    }

    GuiSettings {
        id: guiSettings
    }
}
