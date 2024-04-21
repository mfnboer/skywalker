import QtQuick
import QtQuick.Controls
import skywalker

ComboBox {
    required model
    property bool reversedColors: false

    id: languageComboBox
    height: 34
    valueRole: "shortCode"
    textRole: "shortCode"
    popup.width: 220

    background: Rectangle {
        implicitWidth: 46
        implicitHeight: 34
        border.color: guiSettings.buttonColor
        border.width: 2
        color: reversedColors ? guiSettings.buttonColor : "transparent"
    }

    indicator: Item {}

    contentItem: Text {
        leftPadding: 10
        rightPadding: 10
        verticalAlignment: Text.AlignVCenter
        color: reversedColors ? "white" : guiSettings.buttonColor
        text: languageComboBox.displayText
    }

    delegate: ItemDelegate {
        required property int index
        required property language modelData

        id: delegate
        width: popup.width

        contentItem: Text {
            width: parent.width
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
            color: delegate.index === languageComboBox.currentIndex ? guiSettings.buttonColor : guiSettings.textColor
            text: `${delegate.modelData.nativeName} (${delegate.modelData.shortCode})`
        }
    }

    GuiSettings {
        id: guiSettings
    }

    function setLanguage(shortCode) {
        let index = find(shortCode)

        if (index === -1)
            index = find("en")

        if (index > -1) {
            currentIndex = index
            activated(index)
        }
    }
}
