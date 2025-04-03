import QtQuick


ImageAutoRetry {
    property bool zooming: false

    id: img
    transform: Translate { id: imgTranslation }

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

            if (img.scale < 1)
                img.scale = 1

            img.keepInScreen()
        }

        onGrabChanged: (transition, point) => {
            if (transition === PointerDevice.UngrabPassive) {
                img.zooming = img.scale > 1
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
        enabled: img.zooming

        onTranslationChanged: (delta) => {
            imgTranslation.x += delta.x
            imgTranslation.y += delta.y
            img.keepInScreen()
        }
    }

    function getImgSize() {
        let xScale = width / sourceSize.width
        let yScale = height / sourceSize.height
        let s = Math.min(xScale, yScale)
        return Qt.size(sourceSize.width * s, sourceSize.height * s)
    }

    function keepInScreen() {
        let imgSize = img.getImgSize()

        let maxXDrag = (imgSize.width * img.scale - imgSize.width) / 2
        if (imgTranslation.x > maxXDrag)
            imgTranslation.x = maxXDrag
        else if (imgTranslation.x < -maxXDrag)
            imgTranslation.x = -maxXDrag

        let maxYDrag = (imgSize.height * img.scale - imgSize.height) / 2
        if (imgTranslation.y > maxYDrag)
            imgTranslation.y = maxYDrag
        else if (imgTranslation.y < -maxYDrag)
            imgTranslation.y = -maxYDrag
    }

    function getCenter() {
        return Qt.point(width / 2, height / 2)
    }

    function toggleFullScale(scaleX, scaleY) {
        if (!img.zooming) {
            const fullScale = img.sourceSize.width / img.width

            if (fullScale > 1) {
                const imgCenter = img.getCenter()
                imgTranslation.x = (imgCenter.x - scaleX) * fullScale
                imgTranslation.y = (imgCenter.y - scaleY) * fullScale
                img.scale = fullScale
                img.zooming = true
                img.keepInScreen()
            }
        }
        else {
            img.scale = 1
            img.zooming = false
            imgTranslation.x = 0
            imgTranslation.y = 0
        }
    }
}
