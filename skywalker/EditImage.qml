import QtQuick
import skywalker

SkyPage {
    required property string imgSource
    readonly property int cornerSize: 30
    readonly property int cornerBorderWidth: 3
    readonly property int usableHeight: height - header.height - buttonRow.height

    signal cancel
    signal done(string newImgSource)

    id: page
    width: parent.width

    header: SimpleHeader {
        text: qsTr("Edit Image")
        backIsCancel: true
        onBack: cancel()

        SvgPlainButton {
            anchors.right: parent.right
            anchors.top: parent.top
            svg: SvgOutline.check
            accessibleName: qsTr("editing finished")
            onClicked: transformImage()
        }
    }

    footer: DeadFooterMargin {}

    Item {
        width: parent.width
        height: usableHeight

        scale: calcScale()

        function calcScale() {
            let s = 1

            if (img.sidesSwapped) {
                if (boundingRect.height > width)
                    s = width / boundingRect.height

                if (boundingRect.width > height)
                    s = Math.min(s, height / boundingRect.width)
            }
            else {
                if (boundingRect.width > width)
                    s = width / boundingRect.width

                if (boundingRect.height > height)
                    s = Math.min(s, height / boundingRect.height)
            }

            return s
        }

        Image {
            property size startSize: getImgStartSize()
            property int boundingWidth: boundingRect.width
            property int boundingHeight: boundingRect.height
            property double minScale: 1
            property bool zooming: false

            property bool mirrored: false
            property int rotationCount: 0
            readonly property bool sidesSwapped: rotationCount % 2 === 1

            id: img
            // x: (page.width - width) / 2
            // y: (page.usableHeight - height) / 2
            anchors.centerIn: parent
            width: startSize.width // sidesSwapped ? page.usableHeight : page.width
            height: startSize.height // sidesSwapped ? page.width : page.usableHeight
            fillMode: Image.PreserveAspectCrop
            autoTransform: true
            source: imgSource
            scale: 1
            transform: [Translate {
                    id: imgTranslation

                    onXChanged: console.debug("TRANS X:", x)
                }]

            // onSidesSwappedChanged: {
            //     const r = boundingRect.getSwappedCutScale() / boundingRect.getNormalCutScale()

            //     if (sidesSwapped) {
            //         imgTranslation.x *= r
            //         imgTranslation.y *= r
            //     }
            //     else {
            //         imgTranslation.x /= r
            //         imgTranslation.y /= r
            //     }
            // }

            PinchHandler {
                target: null
                rotationAxis.enabled: false
                xAxis.enabled: false
                yAxis.enabled: false

                onScaleChanged: (delta) => {
                    img.updateScale(centroid.position, delta)

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
                    img.translate(delta.x, delta.y)
                    img.keepInScreen()
                }
            }

            function updateScale(center, delta) {
                let dx = (center.x - img.getCenter().x) * img.scale
                let dy = (center.y - img.getCenter().y) * img.scale

                img.scale *= delta
                imgTranslation.x -= dx * delta - dx
                imgTranslation.y -= dy * delta - dy
            }

            function translate(dx, dy) {
                if (img.rotationCount % 4 === 0) {
                    imgTranslation.x += dx * (img.mirrored ? -1 : 1)
                    imgTranslation.y += dy
                }
                else if (img.rotationCount % 4 === 1) {
                    imgTranslation.x -= dy * (img.mirrored ? -1 : 1)
                    imgTranslation.y += dx
                }
                else if (img.rotationCount % 4 === 2) {
                    imgTranslation.x -= dx * (img.mirrored ? -1 : 1)
                    imgTranslation.y -= dy
                }
                else if (img.rotationCount % 4 === 3) {
                    imgTranslation.x += dy * (img.mirrored ? -1 : 1)
                    imgTranslation.y -= dx
                }
            }

            function getNormalScale() {
                const xScale = page.width / sourceSize.width
                const yScale = page.usableHeight / sourceSize.height
                return Math.min(xScale, yScale)
            }

            function getImageNormalSize() {
                const s = getNormalScale()
                return Qt.size(sourceSize.width * s, sourceSize.height * s)
            }

            function getSwappedScale() {
                const xScale = page.usableHeight / sourceSize.width
                const yScale = page.width / sourceSize.height
                return Math.min(xScale, yScale)
            }

            function getImgStartScale() {
                // if (sidesSwapped) {
                //     console.debug("SWAPPED RATIO:", boundingRect.sidesSwappedRatio, (boundingRect.getSwappedCutScale() / boundingRect.getNormalCutScale()))
                //     return getNormalScale() * boundingRect.sidesSwappedRatio
                // }

                return getNormalScale()
            }

            function getImgStartSize() {
                const s = getImgStartScale()
                return Qt.size(sourceSize.width * s, sourceSize.height * s)
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
            property rect cutRect: Qt.rect(0, 0, img.sourceSize.width, img.sourceSize.height)
            readonly property double cutScale: getNormalCutScale() // img.sidesSwapped ? getSwappedCutScale() : getNormalCutScale()
            property double sidesSwappedRatio: 1 // getSwappedCutScale() / getNormalCutScale()
            property int prevWidth: 0
            property int prevHeight: 0

            function getNormalCutScale() {
                const xScale = page.width / cutRect.width
                const yScale = page.usableHeight / cutRect.height
                return Math.min(xScale, yScale)
            }

            function getSwappedCutScale() {
                const xScale = page.usableHeight / cutRect.width
                const yScale = page.width / cutRect.height
                return Math.min(xScale, yScale)
            }

            function getCutRect()
            {
                const s = cutScale
                return Qt.rect(cutRect.x * s, cutRect.y * s, cutRect.width * s, cutRect.height * s)
            }

            id: boundingRect
            // x: (page.width - width) / 2
            // y: (page.usableHeight - height) / 2
            anchors.centerIn: parent
            width: getCutRect().width
            height: getCutRect().height
            color: "yellow"
            opacity: 0.2
            border.color: guiSettings.textColor
            transform: []

            onWidthChanged: fixCorners()
            onHeightChanged: fixCorners()

            function updateCutRect(cutTopLeftX, cutTopLeftY, cutTopRightX, cutTopRightY,
                                   cutBottomLeftX, cutBottomLeftY, cutBottomRightX, cutBottomRightY) {
                const cx = cutTopLeftX / cutScale + cutRect.x
                const cy = cutTopLeftY / cutScale + cutRect.y
                const cutWidth = cutTopRightX - cutTopLeftX
                const cutHeight = cutBottomLeftY - cutTopLeftY
                const cw = cutWidth / cutScale
                const ch = cutHeight / cutScale

                imgTranslation.x += -cutTopLeftX / 2 + (width - cutTopRightX) / 2
                imgTranslation.y += -cutTopLeftY / 2 + (height - cutBottomLeftY) / 2

                const prevCutScale = getNormalCutScale()
                cutRect.x = cx
                cutRect.y = cy
                cutRect.width = cw
                cutRect.height = ch
                const ratio = getNormalCutScale() / prevCutScale

                const center = Qt.point(img.getCenter().x - imgTranslation.x / img.scale, img.getCenter().y - imgTranslation.y / img.scale)
                console.debug("CUT:", cutRect,  Qt.point(cutTopLeftX, cutTopLeftY), Qt.point(cx, cy), "WIDTH:", cutWidth, cw, "RATIO:", ratio, "CENTER:", center, "IMG CENTER:", img.getCenter(), "SCALE:", img.scale, sidesSwappedRatio)
                img.updateScale(center, ratio)
            }

            function handleCut() {
                boundingRect.updateCutRect(topLeftCorner.x, topLeftCorner.y,
                                           topRightCorner.x + cornerSize, topRightCorner.y,
                                           bottomnLeftCorner.x, bottomnLeftCorner.y + cornerSize,
                                           bottomRightCorner.x + cornerSize, bottomRightCorner.y + cornerSize)
                fixCorners()
            }

            function fixCorners() {
                topLeftCorner.x = 0
                topLeftCorner.y = 0

                topRightCorner.x = boundingRect.width - cornerSize
                topRightCorner.y = 0

                bottomnLeftCorner.x = 0
                bottomnLeftCorner.y = boundingRect.height - cornerSize

                bottomRightCorner.x = boundingRect.width - cornerSize
                bottomRightCorner.y = boundingRect.height - cornerSize
            }

            Rectangle {
                id: topLeftCorner
                width: cornerSize
                height: cornerSize
                color: "transparent"
                border.width: cornerBorderWidth
                border.color: guiSettings.textColor

                onXChanged: {
                    if (topLeftMouse.drag.active)
                        bottomnLeftCorner.x = x
                }

                onYChanged: {
                    if (topLeftMouse.drag.active)
                        topRightCorner.y = y
                }

                MouseArea {
                    id: topLeftMouse
                    anchors.fill: parent
                    drag.target: parent
                    drag.minimumX: 0
                    drag.maximumX: topRightCorner.x - cornerSize
                    drag.minimumY: 0
                    drag.maximumY: bottomnLeftCorner.y - cornerSize

                    drag.onActiveChanged: {
                        if (!drag.active) {
                            boundingRect.handleCut()
                        }
                    }
                }
            }

            Rectangle {
                id: topRightCorner
                x: parent.width - width
                width: cornerSize
                height: cornerSize
                color: "transparent"
                border.width: cornerBorderWidth
                border.color: guiSettings.textColor

                onXChanged: {
                    if (topRightMouse.drag.active)
                        bottomRightCorner.x = x
                }

                onYChanged: {
                    if (topRightMouse.drag.active)
                        topLeftCorner.y = y
                }

                MouseArea {
                    id: topRightMouse
                    anchors.fill: parent
                    drag.target: parent
                    drag.minimumX: topLeftCorner.x + cornerSize
                    drag.maximumX: boundingRect.width - cornerSize
                    drag.minimumY: 0
                    drag.maximumY: bottomRightCorner.y - cornerSize

                    drag.onActiveChanged: {
                        if (!drag.active) {
                            boundingRect.handleCut()
                        }
                    }
                }
            }

            Rectangle {
                id: bottomnLeftCorner
                y: boundingRect.height - cornerSize
                width: cornerSize
                height: cornerSize
                color: "transparent"
                border.width: cornerBorderWidth
                border.color: guiSettings.textColor

                onXChanged: {
                    if (bottomLeftMouse.drag.active)
                        topLeftCorner.x =x
                }

                onYChanged: {
                    if (bottomLeftMouse.drag.active)
                        bottomRightCorner.y = y
                }

                MouseArea {
                    id: bottomLeftMouse
                    anchors.fill: parent
                    drag.target: parent
                    drag.minimumX: 0
                    drag.maximumX: bottomRightCorner.x - cornerSize
                    drag.minimumY: topLeftCorner.y + cornerSize
                    drag.maximumY: boundingRect.height - cornerSize

                    drag.onActiveChanged: {
                        if (!drag.active) {
                            boundingRect.handleCut()
                        }
                    }
                }
            }

            Rectangle {
                id: bottomRightCorner
                x: boundingRect.width - cornerSize
                y: boundingRect.height - cornerSize
                width: cornerSize
                height: cornerSize
                color: "transparent"
                border.width: cornerBorderWidth
                border.color: guiSettings.textColor

                onXChanged: {
                    if (bottomRightMouse.drag.active)
                        topRightCorner.x = x
                }

                onYChanged: {
                    if (bottomRightMouse.drag.active)
                        bottomnLeftCorner.y = y
                }

                MouseArea {
                    id: bottomRightMouse
                    anchors.fill: parent
                    drag.target: parent
                    drag.minimumX: bottomnLeftCorner.x + cornerSize
                    drag.maximumX: boundingRect.width - cornerSize
                    drag.minimumY: topRightCorner.y + cornerSize
                    drag.maximumY: boundingRect.height - cornerSize

                    drag.onActiveChanged: {
                        if (!drag.active) {
                            boundingRect.handleCut()
                        }
                    }
                }
            }
        }
    }

    Rectangle {
        anchors.bottom: parent.bottom
        width: parent.width
        height: buttonRow.height
        color: guiSettings.backgroundColor

        Row {
            id: buttonRow
            anchors.horizontalCenter: parent.horizontalCenter

            SvgButton {
                svg: SvgOutline.mirrorHor
                accessibleName: qsTr("mirror horizontally")
                onClicked: page.mirror()
            }

            SvgButton {
                svg: SvgOutline.rotate90ccw
                accessibleName: qsTr("rotate 90 degrees counter clockwise")
                onClicked: page.rotate()
            }
        }
    }

    ImageUtils {
        id: imageUtils
    }

    Component {
        id: mirrorComponent

        Scale {
            origin.x: img.width / 2
            xScale: -1
        }
    }

    Component {
        id: rotateImgageComponent

        Rotation {
            origin.x: img.width / 2
            origin.y: img.height / 2
            angle: -90
        }
    }

    Component {
        id: rotateBoundingRectComponent

        Rotation {
            origin.x: boundingRect.width / 2
            origin.y: boundingRect.height / 2
            angle: -90
        }
    }

    function mirror() {
        img.transform.push(mirrorComponent.createObject(img))
        img.mirrored = !img.mirrored
    }

    function rotate() {
        img.transform.push(rotateImgageComponent.createObject(img))
        boundingRect.transform.push(rotateBoundingRectComponent.createObject(boundingRect))
        ++img.rotationCount
    }

    function transformImage() {
        let transformations = []

        for (const t of img.transform) {
            if (t instanceof Scale)
                transformations.push(QEnums.IMAGE_TRANSFORM_MIRROR)
            else if (t instanceof Rotation)
                transformations.push(QEnums.IMAGE_TRANSFORM_ROTATE)
            else
                console.warn("Unknown transform:", t)
        }

        const newImgSource = imageUtils.transformImage(imgSource, transformations)
        done(newImgSource)
    }
}
