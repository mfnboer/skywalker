import QtQuick
import QtQuick.Shapes

Shape {
    required property string svgPath
    property string color
    property int svgWidth: 960
    property int svgHeight: 960

    id: shape
    y: height

    ShapePath {
        scale: Qt.size(shape.height / svgWidth, shape.height / svgHeight)
        strokeColor: shape.color
        fillColor: shape.color
        PathSvg { path: svgPath }
    }
}
