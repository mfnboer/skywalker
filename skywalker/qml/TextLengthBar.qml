import QtQuick
import QtQuick.Controls

ProgressBar {
    required property var textField

    id: textLengthBar
    anchors.left: parent.left
    anchors.right: parent.right
    from: 0
    to: textField ? Math.max(textField.maxLength, textField.graphemeLength) : 1
    value: textField ? textField.graphemeLength : 0

    contentItem: Item {
        implicitWidth: parent.width
        implicitHeight: 4

        Rectangle {
            width: textLengthBar.visualPosition * parent.width
            height: parent.height
            color: (textField && textField.graphemeLength <= textField.maxLength) ? guiSettings.buttonColor : guiSettings.errorColor
        }
    }

}
