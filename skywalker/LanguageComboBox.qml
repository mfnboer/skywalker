import QtQuick
import QtQuick.Controls
import skywalker

ComboBox {
    property list<language> allLanguages
    property list<language> usedLanguages
    property bool reversedColors: false
    property int radius: 0
    property string borderColor: guiSettings.buttonColor
    property string color: guiSettings.buttonColor
    property bool addAnyLanguage: false
    property language anyLanguage
    property string initialLanguage

    id: languageComboBox
    height: 34
    model: usedLanguages.concat(allLanguages)
    valueRole: "shortCode"
    textRole: "shortCode"
    popup.width: 220

    background: Rectangle {
        implicitWidth: 46
        implicitHeight: 34
        radius: languageComboBox.radius
        border.color: languageComboBox.borderColor
        border.width: 2
        color: reversedColors ? languageComboBox.borderColor : "transparent"
    }

    indicator: Item {}

    contentItem: Text {
        leftPadding: 10
        rightPadding: 10
        verticalAlignment: Text.AlignVCenter
        color: reversedColors ? "white" : languageComboBox.color
        text: languageComboBox.displayText ? languageComboBox.displayText : qsTr("Any lanuage")
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
            text: delegate.modelData.shortCode ? `${delegate.modelData.nativeName} (${delegate.modelData.shortCode})` : qsTr("Any language")
        }

        background: Rectangle {
            implicitWidth: delegate.width
            color: delegate.highlighted ? Material.listHighlightColor : (delegate.index < usedLanguages.length + (addAnyLanguage ? 1 : 0) ? guiSettings.postHighLightColor : "transparent")
        }
    }

    GuiSettings {
        id: guiSettings
    }

    Component.onCompleted: {
        if (addAnyLanguage) {
            let langs = model
            langs.splice(0, 0, anyLanguage)
            model = langs
        }

        if (initialLanguage) {
            currentIndex = find(initialLanguage)
        }
    }
}
