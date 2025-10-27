import QtQuick
import QtQuick.Layouts
import skywalker

Rectangle {
    required property SvgImage svg
    property string iconColor: guiSettgins.textColor

    id: icon
    Layout.preferredWidth: 44
    Layout.preferredHeight: Layout.preferredWidth
    Layout.rowSpan: 2
    color: "transparent"

    SkySvg {
        width: parent.width
        height: width - 12
        x: 6
        y: height + 6
        svg: icon.svg
        color: icon.iconColor
    }
}
