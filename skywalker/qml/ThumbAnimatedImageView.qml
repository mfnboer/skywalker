import QtQuick
import QtQuick.Controls
import skywalker

AnimatedImageAutoRetry {
    required property string url
    property bool showAlt: false

    id: img
    playing: skywalker.getUserSettings().gifAutoPlay
    source: url

    AltLabel {
        visible: showAlt
    }
}
