import QtQuick

SkyCleanedText {
    property string postHighlightColor: "transparent"

    wrapMode: Text.Wrap
    elide: Text.ElideRight
    textFormat: Text.RichText

    Rectangle {
        anchors.fill: parent
        z: parent.z - 2
        radius: 5
        color: postHighlightColor
        opacity: guiSettings.focusHighlightOpacity
    }
}
