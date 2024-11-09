import QtQuick
import QtQuick.Controls

Text {
    required property var textField

    color: textField.graphemeLength <= textField.maxLength ? Material.color(Material.Grey) : GuiSettings.errorColor
    text: textField.maxLength - textField.graphemeLength

    Accessible.role: Accessible.StaticText
    Accessible.name: getSpeech()

    function getSpeech() {
        const charLeft = textField.maxLength - textField.graphemeLength

        if (charLeft >= 0)
            return qsTr(`${charLeft} characters left for your message`)

        return qsTr(`Your message is ${(-charLeft)} characters too long`)
    }

}
