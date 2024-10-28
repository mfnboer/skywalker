import QtQuick
import QtQuick.Controls
import skywalker

RoundButton {
    required property string accessibleName
    required property SvgImage svg
    property string iconColor: guiSettings.buttonTextColor
    property int imageMargin: 10

    id: button
    Material.background: enabled ? guiSettings.buttonColor : guiSettings.disabledColor
    opacity: 1

    Accessible.role: Accessible.Button
    Accessible.name: accessibleName
    Accessible.onPressAction: if (button.enabled) button.clicked()

    SkySvg {
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
