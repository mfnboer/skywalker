import QtQuick
import QtQuick.Controls
import skywalker

AnimatedImageAutoRetry {
    required property string url

    id: img
    playing: skywalker.getUserSettings().gifAutoPlay
    source: url
}
