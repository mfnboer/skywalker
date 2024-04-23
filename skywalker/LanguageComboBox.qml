import QtQuick
import QtQuick.Controls
import skywalker

ComboBox {
    property list<language> allLanguages
    property list<language> usedLanguages
    property bool reversedColors: false

    id: languageComboBox
    height: 34
    model: usedLanguages.concat(allLanguages)
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
        highlighted: languageComboBox.highlightedIndex === index

        contentItem: Text {
            width: delegate.width
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
            color: delegate.index === languageComboBox.currentIndex ? guiSettings.buttonColor : guiSettings.textColor
            text: `${delegate.modelData.nativeName} (${delegate.modelData.shortCode})`
        }

        background: Rectangle {
            implicitWidth: delegate.width
            color: delegate.highlighted ? Material.listHighlightColor : (delegate.index < usedLanguages.length ? guiSettings.postHighLightColor : "transparent")
        }
    }

    GuiSettings {
        id: guiSettings
    }
}