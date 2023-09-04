import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Dialog {
    required property list<imageview> images
    required property int imageIndex

    width: parent.width
    height: parent.height
    background: Rectangle { color: "black" }
    onClosed: destroy()

    SwipeView {
        id: view
        anchors.fill: parent
        currentIndex: imageIndex

        Repeater {
            model: images.length

            Rectangle {
                required property int index
                property bool isCurrentItem: SwipeView.isCurrentItem

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

    RoundButton {
        id: backButton
        anchors.top: view.top
        anchors.left: view.left
        Material.background: "black"
        onClicked: close()

        SvgImage {
            width: 20
            height: 20
            x: parent.x
            svg: svgOutline.arrowBack
            color: "white"
        }
    }
}
