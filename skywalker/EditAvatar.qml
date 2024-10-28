import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Shapes
import skywalker

SkyPage {
    required property string photoSource
    property double relativeRadius: 0.5
    property double maskOpacity: 0.8
    property int maskWidth: getMaskSize()
    property int maskHeight: maskWidth
    property int pageMargin: 10

    signal closed
    signal selected(rect rectangle)

    id: page
    width: parent.width
    height: parent.height
    padding: pageMargin

    Accessible.role: Accessible.Pane

    header: SimpleHeader {
        text: qsTr("Edit Photo")
        backIsCancel: true
        onBack: closed()

        SvgButton {
            anchors.right: parent.right
            anchors.top: parent.top
            svg: SvgOutline.check
            accessibleName: qsTr("save avatar")
            onClicked: selected(img.getSelectRect())
        }
    }

    Image {
        property int boundingWidth: maskWidth
        property int boundingHeight: maskHeight
        property double minScale: 1
        property bool zooming: false

        id: img
        y: (parent.height - helpText.height - height) / 2
        width: parent.width
        height: parent.height - helpText.height
        fillMode: Image.PreserveAspectFit
        source: photoSource
        transform: Translate { id: imgTranslation }

        onWidthChanged: initMinScale()
        onHeightChanged: initMinScale()
        onSourceSizeChanged: initMinScale()

        PinchHandler {
            target: null
            rotationAxis.enabled: false
            xAxis.enabled: false
            yAxis.enabled: false

            onScaleChanged: (delta) => {
                let dx = (centroid.position.x - img.getCenter().x) * img.scale
                let dy = (centroid.position.y - img.getCenter().y) * img.scale

                img.scale *= delta
                imgTranslation.x -= dx * delta - dx
                imgTranslation.y -= dy * delta - dy

                if (img.scale < img.minScale)
                    img.scale = img.minScale

                img.keepInScreen()
            }

            onGrabChanged: (transition, point) => {
                if (transition === PointerDevice.UngrabPassive) {
                    img.zooming = img.scale > img.minScale
                }
            }
        }

        PinchHandler {
            id: imgDrag
            target: null
            rotationAxis.enabled: false
            scaleAxis.enabled: false
            minimumPointCount: 1
            maximumPointCount: 1

            onTranslationChanged: (delta) => {
                imgTranslation.x += delta.x
                imgTranslation.y += delta.y
                img.keepInScreen()
            }
        }

        function initMinScale() {
            if (width === 0 || height === 0 || sourceSize.width === 0 || sourceSize.height === 0)
                return

            img.minScale = 1
            let sz = img.getImgSize()

            if (sz.height < img.boundingHeight)
                img.minScale = img.boundingHeight / sz.height
            else if (sz.width < img.boundingWidth)
                img.minScale = img.boundingWidth / sz.width

            img.scale = img.minScale
        }

        function getImgScale() {
            let xScale = width / sourceSize.width
            let yScale = height / sourceSize.height
            let s = Math.min(xScale, yScale)
            return s
        }

        function getImgSize() {
            let s = getImgScale()
            return Qt.size(sourceSize.width * s, sourceSize.height * s)
        }

        function getCenter() {
            return Qt.point(width / 2, height / 2)
        }

        function keepInScreen() {
            let imgSize = img.getImgSize()

            let maxXDrag = (imgSize.width * img.scale - img.boundingWidth) / 2
            if (imgTranslation.x > maxXDrag)
                imgTranslation.x = maxXDrag
            else if (imgTranslation.x < -maxXDrag)
                imgTranslation.x = -maxXDrag

            let maxYDrag = (imgSize.height * img.scale - img.boundingHeight) / 2
            if (imgTranslation.y > maxYDrag)
                imgTranslation.y = maxYDrag
            else if (imgTranslation.y < -maxYDrag)
                imgTranslation.y = -maxYDrag
        }

        function getSelectRect() {
            console.debug("BOUNDING SIZE:", img.boundingWidth, img.boundingHeight)
            let s = img.getImgScale()
            let imgSize = img.getImgSize()

            let x0 = (imgSize.width * img.scale - img.boundingWidth) / 2
            let y0 = (imgSize.height * img.scale - img.boundingHeight) / 2

            let rx = (x0 - imgTranslation.x) / img.scale / s
            let ry = (y0 - imgTranslation.y) / img.scale / s
            let rw = img.boundingWidth / img.scale / s
            let rh = img.boundingHeight / img.scale / s

            let r = Qt.rect(rx, ry, rw, rh)
            return r
        }
    }

    Rectangle {
        x: -10
        y: -10
        width: parent.width + 20
        height: topLeftMask.y - y
        color: "black"
        opacity: maskOpacity
    }

    Rectangle {
        x: -10
        y: topLeftMask.y
        width: 10
        height: bottomLeftMask.y + bottomLeftMask.height - y
        color: "black"
        opacity: maskOpacity
    }

    Rectangle {
        x: topRightMask.x + topRightMask.width
        y: topRightMask.y
        width: 10
        height: bottomRightMask.y + bottomRightMask.height - y
        color: "black"
        opacity: maskOpacity
    }

    Rectangle {
        x: -10
        y: bottomLeftMask.y + bottomLeftMask.height
        width: parent.width + 20
        height: helpRect.y - y
        color: "black"
        opacity: maskOpacity
    }

    Shape {
        id: topLeftMask
        x: 0
        y: (parent.height - helpText.height - maskHeight) / 2
        width: getRadius()
        height: width

        ShapePath {
            strokeColor: "transparent"
            fillColor: "#CC000000"
            startX: 0; startY: 0

            PathLine { x: getRadius(); y: 0 }
            PathArc {
                x: 0; y: getRadius();
                radiusX: getRadius(); radiusY: getRadius();
                direction: PathArc.Counterclockwise
            }
        }
    }

    Shape {
        id: topRightMask
        x: parent.width - width
        y: topLeftMask.y
        width: getRadius()
        height: width

        ShapePath {
            strokeColor: "transparent"
            fillColor: "#CC000000"
            startX: getRadius(); startY: 0

            PathLine { x: 0; y: 0 }
            PathArc {
                x: getRadius(); y: getRadius();
                radiusX: getRadius(); radiusY: getRadius();
                direction: PathArc.Clockwise
            }
        }
    }

    Shape {
        id: bottomLeftMask
        x: topLeftMask.x
        y: topLeftMask.y + maskHeight - width
        width: getRadius()
        height: width

        ShapePath {
            strokeColor: "transparent"
            fillColor: "#CC000000"
            startX: 0; startY: getRadius()

            PathLine { x: getRadius(); y: getRadius() }
            PathArc {
                x: 0; y: 0;
                radiusX: getRadius(); radiusY: getRadius();
                direction: PathArc.Clockwise
            }
        }
    }

    Shape {
        id: bottomRightMask
        x: topRightMask.x
        y: bottomLeftMask.y
        width: getRadius()
        height: width

        ShapePath {
            strokeColor: "transparent"
            fillColor: "#CC000000"
            startX: getRadius(); startY: getRadius()

            PathLine { x: 0; y: getRadius() }
            PathArc {
                x: getRadius(); y: 0;
                radiusX: getRadius(); radiusY: getRadius();
                direction: PathArc.Counterclockwise
            }
        }
    }

    Rectangle {
        id: helpRect
        x: -10
        width: parent.width + 20
        height: helpText.height + 10
        anchors.bottom: parent.bottom
        anchors.bottomMargin: -10
        color: "black"

        AccessibleText {
            id: helpText
            width: parent.width - 20
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            elide: Text.ElideRight
            font.pointSize: guiSettings.scaledFont(9/8)
            color: "white"
            text: qsTr("Zoom and drag photo into desired position.")
        }
    }

    GuiSettings {
        id: guiSettings
    }

    function getMaskSize() {
        let sz = Math.min(width, height) - 2 * padding
        return sz
    }

    function getRadius() {
        return relativeRadius * getMaskSize()
    }
}
