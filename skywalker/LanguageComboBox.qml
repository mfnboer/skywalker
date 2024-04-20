import QtQuick
import QtQuick.Controls
import skywalker

ComboBox {
    id: languageComboBox
    height: 34
    model: languageUtils.languages
    valueRole: "shortCode"
    textRole: "shortCode"
    popup.width: 220

    background: Rectangle {
        implicitWidth: 46
        implicitHeight: 34
        border.color: guiSettings.buttonColor
        border.width: 2
        color: "transparent"
    }

    indicator: Item {}

    contentItem: Text {
        leftPadding: 10
        rightPadding: 10
        verticalAlignment: Text.AlignVCenter
        text: languageComboBox.displayText
    }

    delegate: ItemDelegate {
        required property int index
        required property Language modelData

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

    LanguageUtils {
        id: languageUtils
    }

    GuiSettings {
        id: guiSettings
    }

    function setLanguage(shortCode) {
        let index = find(shortCode)

        if (index === -1)
            index = find("en")

        if (index > -1)
            currentIndex = index
    }

    Component.onCompleted: {
        languageComboBox.contentItem.color = guiSettings.buttonColor
    }
}
