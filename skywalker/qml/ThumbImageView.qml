import QtQuick
import QtQuick.Controls
import skywalker

ImageAutoRetry {
    required property imageview imageView
    property bool enableAlt: true

    id: img
    source: imageView.thumbUrl
    cache: false
    Accessible.role: Accessible.StaticText // Graphic role does not work??
    Accessible.name: qsTr(`picture: ${imageView.alt}`)

    AltLabel {
        backgroundColor: imageView.hasHtmlAlt() ? guiSettings.hideReasonLabelColor : "black"
        visible: imageView.alt && enableAlt
    }
}
