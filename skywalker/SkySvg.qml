import QtQuick
import QtQuick.Shapes
import skywalker

Shape {
    required property SvgImage svg
    property string color
    property string outlineColor: color

    id: shape
    y: height

    ShapePath {
        scale: Qt.size(shape.height / svg.width, shape.height / svg.height)
        strokeWidth: 0.4
        strokeColor: shape.outlineColor
        fillColor: shape.color
        PathSvg { path: svg.path }
    }
}
