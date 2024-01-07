import QtQuick
import QtQuick.Controls

ProgressBar {
    required property var textField

    id: textLengthBar
    anchors.left: parent.left
    anchors.right: parent.right
    from: 0
    to: Math.max(textField.maxLength, textField.graphemeLength)
    value: textField.graphemeLength

    contentItem: Rectangle {
        width: textLengthBar.visualPosition * parent.width
        height: parent.height
        color: textField.graphemeLength <= textField.maxLength ? guiSettings.buttonColor : guiSettings.errorColor
    }

    GuiSettings {
        id: guiSettings
    }
}
