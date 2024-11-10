import QtQuick
import QtQuick.Controls
import QtQuick.Shapes

Rectangle {
    property int cornerRadius: 10
    property string maskColor: guiSettings.backgroundColor

    id: maskRect
    color: "transparent"

    Shape {
        id: topLeftMask
        x: 0
        y: 0
        width: maskRect.cornerRadius
        height: maskRect.cornerRadius

        ShapePath {
            strokeColor: maskRect.maskColor
            fillColor: maskRect.maskColor
            startX: 0; startY: 0

            PathLine { x: maskRect.cornerRadius; y: 0 }
            PathArc {
                x: 0; y: maskRect.cornerRadius;
                radiusX: maskRect.cornerRadius; radiusY: maskRect.cornerRadius;
                direction: PathArc.Counterclockwise
            }
        }
    }

    Shape {
        id: topRightMask
        x: maskRect.width - width
        y: 0
        width: maskRect.cornerRadius
        height: maskRect.cornerRadius

        ShapePath {
            strokeColor: maskRect.maskColor
            fillColor: maskRect.maskColor
            startX: maskRect.cornerRadius; startY: 0

            PathLine { x: 0; y: 0 }
            PathArc {
                x: maskRect.cornerRadius; y: maskRect.cornerRadius;
                radiusX: maskRect.cornerRadius; radiusY: maskRect.cornerRadius;
                direction: PathArc.Clockwise
            }
        }
    }

    Shape {
        id: bottomLeftMask
        x: 0
        y: maskRect.height - height
        width: maskRect.cornerRadius
        height: maskRect.cornerRadius

        ShapePath {
            strokeColor: maskRect.maskColor
            fillColor: maskRect.maskColor
            startX: 0; startY: maskRect.cornerRadius

            PathLine { x: maskRect.cornerRadius; y: maskRect.cornerRadius }
            PathArc {
                x: 0; y: 0;
                radiusX: maskRect.cornerRadius; radiusY: maskRect.cornerRadius;
                direction: PathArc.Clockwise
            }
        }
    }

    Shape {
        id: bottomRightMask
        x: maskRect.width - width
        y: maskRect.height - height
        width: maskRect.cornerRadius
        height: maskRect.cornerRadius

        ShapePath {
            strokeColor: maskRect.maskColor
            fillColor: maskRect.maskColor
            startX: maskRect.cornerRadius; startY: maskRect.cornerRadius

            PathLine { x: 0; y: maskRect.cornerRadius }
            PathArc {
                x: maskRect.cornerRadius; y: 0
                radiusX: maskRect.cornerRadius; radiusY: maskRect.cornerRadius;
                direction: PathArc.Counterclockwise
            }
        }
    }

}
