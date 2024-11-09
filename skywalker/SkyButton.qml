import QtQuick
import QtQuick.Controls

RoundButton {
    id: button

    Material.background: flat ? GuiSettings.labelColor : GuiSettings.buttonColor

    contentItem: Text {
        leftPadding: 10
        rightPadding: 10
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        color: flat ? GuiSettings.textColor : GuiSettings.buttonTextColor
        text: button.text
    }

    Accessible.role: Accessible.Button
    Accessible.name: button.text
    Accessible.onPressAction: if (enabled) clicked()

}
