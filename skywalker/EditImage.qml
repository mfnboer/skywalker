import QtQuick
import skywalker

SkyPage {
    required property string imgSource
    readonly property int cornerSize: 35
    readonly property int cornerBorderWidth: 2
    readonly property int cornerMargin: 5
    readonly property double maskOpacity: 0.6
    readonly property string cutToolColor: guiSettings.buttonColor
    readonly property string maskColor: guiSettings.backgroundColor
    readonly property int usableHeight: height - header.height - buttonRow.height - footer.height

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
        id: editItem
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

            property bool horMirrored: false
            property bool vertMirrored: false
            property int rotationCount: 0
            readonly property bool sidesSwapped: rotationCount % 2 === 1
            readonly property int maxXDrag: getMaxXDrag()
            readonly property int maxYDrag: getMaxYDrag()

            id: img
            anchors.centerIn: parent
            width: startSize.width
            height: startSize.height
            fillMode: Image.PreserveAspectCrop
            autoTransform: true
            source: imgSource
            scale: 1
            transform: [
                Translate {
                    id: imgTranslation
                },
                Scale {
                    origin.x: img.width / 2
                    xScale: img.horMirrored ? -1 : 1
                },
                Scale {
                    origin.y: img.height / 2
                    yScale: img.vertMirrored ? -1 : 1
                },
                Rotation {
                    id: imgRotation
                    origin.x: img.width / 2
                    origin.y: img.height / 2
                    angle: img.rotationCount * -90
                }
            ]

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

            function reset() {
                boundingRect.resetSize()
                scale = minScale
                imgTranslation.x = 0
                imgTranslation.y = 0
                rotationCount = 0
                horMirrored = false
                vertMirrored = false
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
                    imgTranslation.x += dx * (img.horMirrored ? -1 : 1)
                    imgTranslation.y += dy * (img.vertMirrored ? -1 : 1)
                }
                else if (img.rotationCount % 4 === 1) {
                    imgTranslation.x -= dy * (img.horMirrored ? -1 : 1)
                    imgTranslation.y += dx * (img.vertMirrored ? -1 : 1)
                }
                else if (img.rotationCount % 4 === 2) {
                    imgTranslation.x -= dx * (img.horMirrored ? -1 : 1)
                    imgTranslation.y -= dy * (img.vertMirrored ? -1 : 1)
                }
                else if (img.rotationCount % 4 === 3) {
                    imgTranslation.x += dy * (img.horMirrored ? -1 : 1)
                    imgTranslation.y -= dx * (img.vertMirrored ? -1 : 1)
                }
            }

            function calcImgStartScale() {
                const xScale = (page.width - 2 * cornerSize) / sourceSize.width
                const yScale = (page.usableHeight - 2 * cornerSize) / sourceSize.height
                return Math.min(xScale, yScale)
            }

            function getImgStartSize() {
                const s = calcImgStartScale()
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

            function getMaxXDrag() {
                let imgSize = img.getImgSize()
                return (imgSize.width * img.scale - img.boundingWidth) / 2
            }

            function getMaxYDrag() {
                let imgSize = img.getImgSize()
                return (imgSize.height * img.scale - img.boundingHeight) / 2
            }

            function keepInScreen() {
                if (img.scale < img.minScale)
                    img.scale = img.minScale

                if (imgTranslation.x > maxXDrag)
                    imgTranslation.x = maxXDrag
                else if (imgTranslation.x < -maxXDrag)
                    imgTranslation.x = -maxXDrag

                if (imgTranslation.y > maxYDrag)
                    imgTranslation.y = maxYDrag
                else if (imgTranslation.y < -maxYDrag)
                    imgTranslation.y = -maxYDrag

                let s = 1

                if (width * img.scale < boundingWidth)
                    s = boundingWidth / (width * img.scale)

                if (height * img.scale < boundingHeight)
                    s = boundingHeight / (height * img.scale)

                img.scale *= s
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
            readonly property double cutScale: calcCutScale()

            function calcCutScale() {
                const xScale = (page.width - 2 * cornerSize) / cutRect.width
                const yScale = (page.usableHeight - 2 * cornerSize) / cutRect.height
                return Math.min(xScale, yScale)
            }

            function getCutRect()
            {
                const s = cutScale
                return Qt.rect(cutRect.x * s, cutRect.y * s, cutRect.width * s, cutRect.height * s)
            }

            id: boundingRect
            anchors.centerIn: parent
            width: getCutRect().width
            height: getCutRect().height
            color: "transparent"
            border.color: guiSettings.textColor
            border.width: 1
            transform: [
                Rotation {
                    id: boundingRotation
                    origin.x: boundingRect.width / 2
                    origin.y: boundingRect.height / 2
                    angle: img.rotationCount * -90
                }
            ]

            onWidthChanged: fixCorners()
            onHeightChanged: fixCorners()

            function resetSize() {
                cutRect.width = img.sourceSize.width
                cutRect.height = img.sourceSize.height
            }

            function updateCutRect(cutTopLeftX, cutTopLeftY, cutTopRightX, cutTopRightY,
                                   cutBottomLeftX, cutBottomLeftY, cutBottomRightX, cutBottomRightY) {
                const cx = cutTopLeftX / cutScale + cutRect.x
                const cy = cutTopLeftY / cutScale + cutRect.y
                const cutWidth = cutTopRightX - cutTopLeftX
                const cutHeight = cutBottomLeftY - cutTopLeftY
                const cw = cutWidth / cutScale
                const ch = cutHeight / cutScale

                imgTranslation.x += (-cutTopLeftX / 2 + (width - cutTopRightX) / 2) * (img.horMirrored ? -1 : 1)
                imgTranslation.y += (-cutTopLeftY / 2 + (height - cutBottomLeftY) / 2) * (img.vertMirrored ? -1 : 1)

                const prevCutScale = cutScale
                cutRect.x = cx
                cutRect.y = cy
                cutRect.width = cw
                cutRect.height = ch
                const ratio = cutScale / prevCutScale

                const center = Qt.point(img.getCenter().x - imgTranslation.x / img.scale, img.getCenter().y - imgTranslation.y / img.scale)
                console.debug("CUT:", cutRect,  Qt.point(cutTopLeftX, cutTopLeftY), Qt.point(cx, cy), "WIDTH:", cutWidth, cw, "RATIO:", ratio, "CENTER:", center, "IMG CENTER:", img.getCenter(), "SCALE:", img.scale)
                img.updateScale(center, ratio)
            }

            function handleCut() {
                boundingRect.updateCutRect(topLeftCorner.x, topLeftCorner.y,
                                           topRightCorner.x + cornerSize, topRightCorner.y,
                                           bottomLeftCorner.x, bottomLeftCorner.y + cornerSize,
                                           bottomRightCorner.x + cornerSize, bottomRightCorner.y + cornerSize)
                fixCorners()
                img.keepInScreen()
            }

            function fixCorners() {
                topLeftCorner.x = 0
                topLeftCorner.y = 0

                topRightCorner.x = boundingRect.width - cornerSize
                topRightCorner.y = 0

                bottomLeftCorner.x = 0
                bottomLeftCorner.y = boundingRect.height - cornerSize

                bottomRightCorner.x = boundingRect.width - cornerSize
                bottomRightCorner.y = boundingRect.height - cornerSize
            }

            function getHeightForMask() {
                return (img.sidesSwapped ? width : height) * editItem.scale
            }

            function getExtraTopHeightForMask() {
                if (img.rotationCount % 4 == 0)
                    return topLeftCorner.y
                if (img.rotationCount % 4 == 1)
                    return width - topRightCorner.x - cornerSize
                if (img.rotationCount % 4 == 2)
                    return height - bottomRightCorner.y - cornerSize
                if (img.rotationCount % 4 == 3)
                    return bottomLeftCorner.x
            }

            function getExtraBottomHeightForMask() {
                if (img.rotationCount % 4 == 0)
                    return height - bottomLeftCorner.y - cornerSize
                if (img.rotationCount % 4 == 1)
                    return topLeftCorner.x
                if (img.rotationCount % 4 == 2)
                    return topRightCorner.y
                if (img.rotationCount % 4 == 3)
                    return width - bottomRightCorner.x - cornerSize
            }

            function getWidthForMask() {
                return (img.sidesSwapped ? height : width) * editItem.scale
            }

            function getExtraLeftWidthForMask() {
                if (img.rotationCount % 4 == 0)
                    return topLeftCorner.x
                if (img.rotationCount % 4 == 1)
                    return topRightCorner.y
                if (img.rotationCount % 4 == 2)
                    return width - bottomRightCorner.x - cornerSize
                if (img.rotationCount % 4 == 3)
                    return height - bottomLeftCorner.y - cornerSize
            }

            function getExtraRightWidthForMask() {
                if (img.rotationCount % 4 == 0)
                    return width - topRightCorner.x - cornerSize
                if (img.rotationCount % 4 == 1)
                    return height - bottomRightCorner.y - cornerSize
                if (img.rotationCount % 4 == 2)
                    return bottomLeftCorner.x
                if (img.rotationCount % 4 == 3)
                    return topLeftCorner.y
            }

            Rectangle {
                id: topLeftCorner
                width: cornerSize
                height: cornerSize
                color: "transparent"

                Rectangle {
                    anchors.top: parent.top
                    anchors.topMargin: cornerMargin
                    anchors.left: parent.left
                    anchors.leftMargin: cornerMargin
                    width: parent.width - cornerMargin
                    height: cornerBorderWidth
                    color: cutToolColor
                }

                Rectangle {
                    anchors.top: parent.top
                    anchors.topMargin: cornerMargin
                    anchors.left: parent.left
                    anchors.leftMargin: cornerMargin
                    width: cornerBorderWidth
                    height: parent.height - cornerMargin
                    color: cutToolColor
                }

                onXChanged: {
                    if (topLeftMouse.drag.active)
                        bottomLeftCorner.x = x
                }

                onYChanged: {
                    if (topLeftMouse.drag.active)
                        topRightCorner.y = y
                }

                MouseArea {
                    id: topLeftMouse
                    anchors.fill: parent
                    drag.target: parent
                    drag.minimumX: imgTranslation.x - img.maxXDrag
                    drag.maximumX: topRightCorner.x - cornerSize
                    drag.minimumY: imgTranslation.y - img.maxYDrag
                    drag.maximumY: bottomLeftCorner.y - cornerSize

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

                Rectangle {
                    anchors.top: parent.top
                    anchors.topMargin: cornerMargin
                    anchors.right: parent.right
                    anchors.rightMargin: cornerMargin
                    width: parent.width - cornerMargin
                    height: cornerBorderWidth
                    color: cutToolColor
                }

                Rectangle {
                    anchors.top: parent.top
                    anchors.topMargin: cornerMargin
                    anchors.right: parent.right
                    anchors.rightMargin: cornerMargin
                    width: cornerBorderWidth
                    height: parent.height - cornerMargin
                    color: cutToolColor
                }

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
                    drag.maximumX: boundingRect.width - cornerSize + img.maxXDrag + imgTranslation.x
                    drag.minimumY: imgTranslation.y - img.maxYDrag
                    drag.maximumY: bottomRightCorner.y - cornerSize

                    drag.onActiveChanged: {
                        if (!drag.active) {
                            boundingRect.handleCut()
                        }
                    }
                }
            }

            Rectangle {
                id: bottomLeftCorner
                y: boundingRect.height - cornerSize
                width: cornerSize
                height: cornerSize
                color: "transparent"

                Rectangle {
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: cornerMargin
                    anchors.left: parent.left
                    anchors.leftMargin: cornerMargin
                    width: parent.width - cornerMargin
                    height: cornerBorderWidth
                    color: cutToolColor
                }

                Rectangle {
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: cornerMargin
                    anchors.left: parent.left
                    anchors.leftMargin: cornerMargin
                    width: cornerBorderWidth
                    height: parent.height - cornerMargin
                    color: cutToolColor
                }

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
                    drag.minimumX: imgTranslation.x - img.maxXDrag
                    drag.maximumX: bottomRightCorner.x - cornerSize
                    drag.minimumY: topLeftCorner.y + cornerSize
                    drag.maximumY: boundingRect.height - cornerSize + img.maxYDrag + imgTranslation.y

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

                Rectangle {
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: cornerMargin
                    anchors.right: parent.right
                    anchors.rightMargin: cornerMargin
                    width: parent.width - cornerMargin
                    height: cornerBorderWidth
                    color: cutToolColor
                }

                Rectangle {
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: cornerMargin
                    anchors.right: parent.right
                    anchors.rightMargin: cornerMargin
                    width: cornerBorderWidth
                    height: parent.height - cornerMargin
                    color: cutToolColor
                }

                onXChanged: {
                    if (bottomRightMouse.drag.active)
                        topRightCorner.x = x
                }

                onYChanged: {
                    if (bottomRightMouse.drag.active)
                        bottomLeftCorner.y = y
                }

                MouseArea {
                    id: bottomRightMouse
                    anchors.fill: parent
                    drag.target: parent
                    drag.minimumX: bottomLeftCorner.x + cornerSize
                    drag.maximumX: boundingRect.width - cornerSize + img.maxXDrag + imgTranslation.x
                    drag.minimumY: topRightCorner.y + cornerSize
                    drag.maximumY: boundingRect.height - cornerSize + img.maxYDrag + imgTranslation.y

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
        id: topMask
        anchors.top: parent.top
        height: Math.floor((page.usableHeight - boundingRect.getHeightForMask()) / 2) + boundingRect.getExtraTopHeightForMask()
        anchors.left: parent.left
        anchors.right: parent.right
        color: maskColor
        opacity: maskOpacity
    }

    Rectangle {
        id: bottomMask
        height: Math.floor((page.usableHeight - boundingRect.getHeightForMask()) / 2) + boundingRect.getExtraBottomHeightForMask()
        anchors.bottom: buttonRect.top
        anchors.left: parent.left
        anchors.right: parent.right
        color: maskColor
        opacity: maskOpacity
    }

    Rectangle {
        id: leftMask
        anchors.top: topMask.bottom
        anchors.bottom: bottomMask.top
        anchors.left: parent.left
        width: Math.floor((parent.width - boundingRect.getWidthForMask()) / 2) + boundingRect.getExtraLeftWidthForMask()
        color: maskColor
        opacity: maskOpacity
    }

    Rectangle {
        id: rightMask
        anchors.top: topMask.bottom
        anchors.bottom: bottomMask.top
        anchors.right: parent.right
        width: Math.floor((parent.width - boundingRect.getWidthForMask()) / 2) + boundingRect.getExtraRightWidthForMask()
        color: maskColor
        opacity: maskOpacity
    }

    Rectangle {
        id: buttonRect
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
                onClicked: page.rotate(1)
            }

            SvgButton {
                svg: SvgOutline.rotate90cw
                accessibleName: qsTr("rotate 90 degrees clockwise")
                onClicked: page.rotate(-1)
            }

            SvgButton {
                svg: SvgOutline.resetImage
                accessibleName: qsTr("reset image")
                onClicked: img.reset()
            }
        }
    }

    ImageUtils {
        id: imageUtils
    }

    function mirror() {
        if (!img.sidesSwapped)
            img.horMirrored = !img.horMirrored
        else
            img.vertMirrored = !img.vertMirrored
    }

    function rotate(count) {
        img.rotationCount = (img.rotationCount + count) % 4

        if (img.rotationCount < 0)
            img.rotationCount += 4
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
