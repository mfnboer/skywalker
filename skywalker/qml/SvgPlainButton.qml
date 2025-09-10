import QtQuick
import QtQuick.Controls.Material

SvgButton {
    iconColor: enabled ? guiSettings.textColor : guiSettings.disabledColor
    Material.background: "transparent"
    flat: true
}
