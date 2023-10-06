import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Page {
    required property var images // list<imageview>: var to allow regular javascript arrays
    required property int imageIndex
    signal closed

    id: page
    width: parent.width
    height: parent.height
    padding: 10
    background: Rectangle { color: "black" }

    SwipeView {
        id: view
        anchors.fill: parent
        currentIndex: imageIndex

        Repeater {
            model: images.length

            Rectangle {
                required property int index
                property bool isCurrentItem: SwipeView.isCurrentItem

                id: imgRect
                color: "black"

                Image {
                    id: img
                    y: (parent.height - altText.height - height) / 2
                    width: parent.width
                    height: parent.height - altText.height
                    fillMode: Image.PreserveAspectFit
                    source: images[index].fullSizeUrl
                    transform: Translate { id: imgTranslation }

                    PinchHandler {
                        target: null
                        rotationAxis.enabled: false
                        xAxis.enabled: false
                        yAxis.enabled: false

                        onScaleChanged: (delta) => {
                            img.scale *= delta

                            if (img.scale < 1)
                                img.scale = 1

                            img.keepInScreen()
                        }

                        onGrabChanged: (transition, point) => {
                            if (transition === PointerDevice.UngrabPassive) {
                                view.interactive = img.scale === 1
                                imgDrag.enabled = !view.interactive
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
                        enabled: false

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
                }
                Text {
                    id: altText
                    width: parent.width
                    anchors.bottom: parent.bottom
                    wrapMode: Text.Wrap
                    elide: Text.ElideRight
                    color: "white"
                    text: images[index].alt
                    visible: images[index].alt && isCurrentItem
                }
            }
        }
    }

    SvgButton {
        iconColor: "white"
        Material.background: "black"
        opacity: 0.7
        svg: svgOutline.arrowBack
        onClicked: page.closed()
    }
}
