import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

RoundButton {
    property string textColor: guiSettings.buttonTextColor
    property int textPadding: 10

    id: button

    Material.background: flat ? guiSettings.buttonFlatColor : guiSettings.buttonColor
    display: AbstractButton.TextOnly
    icon.name: ""
    icon.source: ""

    contentItem: Text {
        leftPadding: button.textPadding
        rightPadding: button.textPadding
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
