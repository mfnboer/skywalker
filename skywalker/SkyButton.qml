import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

RoundButton {
    property string textColor: guiSettings.buttonTextColor

    id: button

    Material.background: flat ? guiSettings.buttonFlatColor : guiSettings.buttonColor
    display: AbstractButton.TextOnly
    icon.name: ""
    icon.source: ""

    contentItem: Text {
        leftPadding: 10
        rightPadding: 10
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        color: textColor
        text: button.text
        font: button.font
    }

    Accessible.role: Accessible.Button
    Accessible.name: button.text
    Accessible.onPressAction: if (enabled) clicked()
}
