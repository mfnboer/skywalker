import QtQuick
import QtQuick.Controls

RoundButton {
    id: button

    Material.background: flat ? guiSettings.labelColor : guiSettings.buttonColor

    contentItem: Text {
        leftPadding: 10
        rightPadding: 10
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        color: flat ? guiSettings.textColor : guiSettings.buttonTextColor
        text: button.text
    }

    Accessible.role: Accessible.Button
    Accessible.name: button.text
    Accessible.onPressAction: if (enabled) clicked()

}
