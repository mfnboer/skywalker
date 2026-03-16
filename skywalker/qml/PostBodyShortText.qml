import QtQuick
import skywalker

AccessibleText {
    required property textmetainfo postTextMetaInfo
    property string postHighlightColor: "transparent"
    property bool hasThreadIndicator: false

    wrapMode: Text.Wrap
    textFormat: isSimpleText() ? Text.StyledText : Text.RichText

    Rectangle {
        anchors.fill: parent
        z: parent.z - 2
        radius: 5
        color: postHighlightColor
        opacity: guiSettings.focusHighlightOpacity
    }

    function isSimpleText() {
        return !postTextMetaInfo.hasFacets &&
                !postTextMetaInfo.hasCombinedEmoji &&
                !postTextMetaInfo.hasContinuousWhitespace &&
                !hasThreadIndicator
    }
}
