import QtQuick

Text {
    required property var textField

    color: textField.graphemeLength <= textField.maxLength ? guiSettings.textColor : guiSettings.errorColor
    text: textField.maxLength - textField.graphemeLength

    Accessible.role: Accessible.StaticText
    Accessible.name: getSpeech()

    function getSpeech() {
        const charLeft = textField.maxLength - textField.graphemeLength

        if (charLeft >= 0)
            return qsTr(`${charLeft} characters left for your message`)

        return qsTr(`Your message is ${(-charLeft)} characters too long`)
    }

    GuiSettings {
        id: guiSettings
    }
}
