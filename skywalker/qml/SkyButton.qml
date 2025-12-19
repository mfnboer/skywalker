import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

RoundButton {
    property string textColor: guiSettings.buttonTextColor
    property int textPadding: 10

    id: button

    height: implicitHeight ? Math.max(implicitHeight, guiSettings.appFontHeight + 10) : undefined
    Material.background: flat ? guiSettings.buttonFlatColor : guiSettings.buttonColor
    display: AbstractButton.TextOnly
    icon.name: ""
    icon.source: ""
    font.pointSize: guiSettings.scaledFont(7/8)
    font.weight: Font.Medium

    contentItem: Text {
        leftPadding: button.textPadding
        rightPadding: button.textPadding
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        color: textColor
        font: button.font
        text: button.text
    }

    Accessible.role: Accessible.Button
    Accessible.name: button.text
    Accessible.onPressAction: if (enabled) clicked()
}
