import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Page {
    required property list<imageview> images
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
                    width: parent.width
                    anchors.top: parent.top
                    anchors.bottom: altText.top
                    fillMode: Image.PreserveAspectFit
                    source: images[index].fullSizeUrl
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
        svg: svgOutline.arrowBack
    }
}
