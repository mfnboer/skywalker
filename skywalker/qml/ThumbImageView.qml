import QtQuick
import QtQuick.Controls
import skywalker

ImageAutoRetry {
    required property imageview imageView
    property bool enableAlt: true

    id: img
    source: imageView.thumbUrl
    Accessible.role: Accessible.StaticText // Graphic role does not work??
    Accessible.name: qsTr(`picture: ${imageView.alt}`)

    Label {
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        leftInset: 4
        rightInset: 4
        topInset: 5
        bottomInset: 5
        padding: 5
        color: "white"
        font.pointSize: guiSettings.scaledFont(4.5/8)
        text: qsTr("ALT", "alternative text indication on an image")
        visible: imageView.alt && enableAlt

        background: Rectangle {
            radius: 3
            color: "black"
            opacity: 0.6
        }
    }

}
