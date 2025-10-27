import QtQuick
import QtQuick.Shapes

Rectangle {
    property int cornerRadius: guiSettings.radius
    property string maskColor: guiSettings.backgroundColor
    property int maskWidth: width
    property int maskHeight: height

    id: maskRect
    color: "transparent"

    Shape {
        id: topLeftMask
        x: (maskRect.width - maskRect.maskWidth) / 2 - 0.5
        y: (maskRect.height - maskRect.maskHeight) / 2 - 0.5
        z: parent.z + 2
        width: maskRect.cornerRadius
        height: maskRect.cornerRadius
        visible: cornerRadius > 0

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
        x: maskRect.maskWidth + topLeftMask.x - width + 1
        y: topLeftMask.y
        z: parent.z + 2
        width: maskRect.cornerRadius
        height: maskRect.cornerRadius
        visible: cornerRadius > 0

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
        x: topLeftMask.x
        y: maskRect.maskHeight + topLeftMask.y - height + 1
        z: parent.z + 2
        width: maskRect.cornerRadius
        height: maskRect.cornerRadius
        visible: cornerRadius > 0

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
        x: topRightMask.x
        y: bottomLeftMask.y
        z: parent.z + 2
        width: maskRect.cornerRadius
        height: maskRect.cornerRadius
        visible: cornerRadius > 0

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
