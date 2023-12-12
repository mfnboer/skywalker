import QtQuick

TextEdit {
    property string placeholderText

    id: skyTextEdit
    width: page.width
    leftPadding: 10
    rightPadding: 10
    textFormat: TextEdit.PlainText
    wrapMode: TextEdit.Wrap
    font.pointSize: guiSettings.scaledFont(9/8)
    color: guiSettings.textColor
    selectionColor: guiSettings.selectionColor
    clip: true
    focus: true

    Text {
        anchors.fill: parent
        leftPadding: skyTextEdit.leftPadding
        rightPadding: skyTextEdit.rightPadding
        font.pointSize: guiSettings.scaledFont(7/8)
        color: guiSettings.placeholderTextColor
        elide: Text.ElideRight
        text: placeholderText
        visible: skyTextEdit.length + skyTextEdit.preeditText.length === 0
    }

    GuiSettings {
        id: guiSettings
    }
}
