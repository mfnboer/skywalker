import QtQuick

SkyCleanedText {
    required property string alt
    property bool isHtml: false
    readonly property int bottomMargin: 10 + guiSettings.footerMargin
    readonly property int maxHeight: 6 * 21

    leftPadding: 10
    width: parent.width - 15
    wrapMode: Text.Wrap
    color: "white"
    plainText: alt
    textFormat: isHtml ? Text.RichText : Text.PlainText
}
