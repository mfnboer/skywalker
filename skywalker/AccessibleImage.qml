import QtQuick

// Hack to make Talkback work. Rounding of the image corners seems to break it.
Item {
    required property var image
    required property string alt

    x: image.x
    y: image.y
    width: image.width
    height: image.height

    Accessible.role: Accessible.StaticText // Graphic role does not work??
    Accessible.name: qsTr(`picture: ${alt}`)
}
