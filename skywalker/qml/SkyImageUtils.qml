import QtQuick
import skywalker

ImageUtils {
    id: imageUtils

    function setDominantColor(img, cb = (color) => {}) {
        const cutX = Math.max((img.width - img.paintedWidth) / 2, 0)
        const cutY = Math.max((img.height - img.paintedHeight) / 2, 0)
        const cutWidth = Math.min(img.paintedWidth, img.width - cutX)
        const cutHeight = Math.min(img.paintedHeight, img.height - cutY)
        const cutRect = Qt.rect(cutX * Screen.devicePixelRatio, cutY * Screen.devicePixelRatio,
                                cutWidth * Screen.devicePixelRatio, cutHeight * Screen.devicePixelRatio)

        console.debug("Grab image:", img.width, img.height)

        img.grabToImage((result) => {
            const color = imageUtils.getDominantColor(result.image, cutRect, 16 * Screen.devicePixelRatio)
            cb(color)
        })
    }
}
