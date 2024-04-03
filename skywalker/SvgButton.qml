import QtQuick
import QtQuick.Controls
import skywalker

RoundButton {
    id: button
    required property svgimage svg
    property string iconColor: guiSettings.buttonTextColor
    property int imageMargin: 10

    Material.background: enabled ? guiSettings.buttonColor : guiSettings.disabledColor
    opacity: 1

    SvgImage {
        width: button.width - 2 * button.imageMargin
        height: button.height - 2 * button.imageMargin
        x: button.imageMargin
        y: height + button.imageMargin
        svg: button.svg
        color: button.iconColor
        Accessible.ignored: true
    }

    GuiSettings {
        id: guiSettings
    }
}
