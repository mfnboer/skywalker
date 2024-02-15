import QtQuick
import skywalker

TextEdit {
    property string placeholderText
    property double placeholderPointSize: guiSettings.scaledFont(7/8)

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

    Accessible.role: Accessible.EditableText
    Accessible.name: placeholder.visible ? placeholderText : text
    Accessible.multiLine: true
    Accessible.editable: true

    Text {
        id: placeholder
        anchors.fill: parent
        padding: parent.padding
        leftPadding: parent.leftPadding
        rightPadding: parent.rightPadding
        font.pointSize: placeholderPointSize
        color: guiSettings.placeholderTextColor
        elide: Text.ElideRight
        text: placeholderText
        visible: skyTextEdit.length + skyTextEdit.preeditText.length === 0
    }

    UnicodeFonts {
        id: unicodeFonts
    }

    GuiSettings {
        id: guiSettings
    }

    Component.onCompleted: {
        unicodeFonts.setEmojiFixDocument(textDocument)
    }
}
