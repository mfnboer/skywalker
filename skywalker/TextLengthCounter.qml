import QtQuick

Text {
    required property var textField

    color: textField.graphemeLength <= textField.maxLength ? guiSettings.textColor : guiSettings.errorColor
    text: textField.maxLength - textField.graphemeLength

    GuiSettings {
        id: guiSettings
    }
}
