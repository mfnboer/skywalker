import QtQuick

SkyCleanedText {
    required property string alt
    readonly property int bottomMargin: 10 + guiSettings.footerMargin
    readonly property int maxHeight: 6 * 21

    leftPadding: 10
    width: parent.width - 15
    wrapMode: Text.Wrap
    color: "white"
    plainText: alt
}
