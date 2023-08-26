import QtQuick
import QtQuick.Layouts

RoundedImage {
    property list<var> images // ImageView

    width: parent.width
    Layout.fillWidth: true
    source: images[0].thumbUrl
    fillMode: Image.PreserveAspectFit
}
