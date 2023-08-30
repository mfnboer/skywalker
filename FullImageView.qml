import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Dialog {
    required property list<imageview> images
    required property int imageIndex

    width: parent.width
    height: parent.height
    standardButtons: Dialog.Close
    background: Rectangle {
        color: "black"
    }

    SwipeView {
        anchors.fill: parent
        currentIndex: imageIndex

        Repeater {
            model: images.length

            Rectangle {
                required property int index

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
                    maximumLineCount: 3
                    elide: Text.ElideRight
                    color: "white"
                    text: images[index].alt
                    visible: images[index].alt
                }
            }
        }
    }
}
