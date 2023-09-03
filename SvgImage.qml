import QtQuick
import QtQuick.Shapes
import skywalker

Shape {
    required property svgimage svg
    property string color

    id: shape
    y: height

    ShapePath {
        scale: Qt.size(shape.height / svg.width, shape.height / svg.height)
        strokeColor: shape.color
        fillColor: shape.color
        PathSvg { path: svg.path }
    }
}
