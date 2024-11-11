import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

RoundButton {
    id: button

    Material.background: flat ? guiSettings.buttonFlatColor : guiSettings.buttonColor

    contentItem: Text {
        leftPadding: 10
        rightPadding: 10
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        color: guiSettings.buttonTextColor
        text: button.text
    }

    Accessible.role: Accessible.Button
    Accessible.name: button.text
    Accessible.onPressAction: if (enabled) clicked()

}
