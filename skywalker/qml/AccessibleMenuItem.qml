import QtQuick
import QtQuick.Controls
import skywalker

MenuItem {
    property string textColor: enabled ? guiSettings.textColor : guiSettings.disabledColor
    property SvgImage svg
    property string svgColor: textColor

    id: control
    width: parent.width
    implicitHeight: visible ? Math.max(50, guiSettings.appFontHeight + 10) : 0

    contentItem: Row {
        readonly property real indicatorPadding: control.checkable && control.indicator ? control.indicator.width + control.spacing : 0

        width: control.width
        height: control.height

        Text {
            y: (parent.height - height) / 2
            width: parent.width - icon.width - leftPadding - rightPadding - 10
            leftPadding: parent.indicatorPadding
            rightPadding: 10
            text: control.text
            color: control.textColor
            font.pointSize: guiSettings.scaledFont(1)
            elide: Text.ElideRight
        }

        Loader {
            id: icon
            y: -5
            width: height
            height: active ? control.height - 10 : 0
            active: Boolean(control.svg)

            sourceComponent: SkySvg {
                svg: control.svg
                color: control.svgColor
            }
        }
    }

    Accessible.role: Accessible.MenuItem
    Accessible.name: text
    Accessible.description: Accessible.name
    Accessible.onPressAction: triggered()
}
