import QtQuick

AccessibleText {
    width: parent.width
    wrapMode: Text.Wrap
    textFormat: Text.RichText
    text: '<a href="show" style="color: ${guiSettings.linkColor};">' + qsTr("Show author") + '</a>'
}
