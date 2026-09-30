import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

ColumnLayout {
    property var skywalker: root.getSkywalker()
    property var userSettings: skywalker.getUserSettings()
    property string userDid: userSettings.getActiveUserDid()

    id: column

    HeaderText {
        Layout.topMargin: 10
        text: qsTr("Language preferences")
    }
    AccessibleText {
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        text: qsTr("Which languages do you want to see in feeds (other than home). If no languages are selected than all posts will be shown. Note that the language tags on posts may be wrong or missing.")
    }

    LanguageComboCheckBox {
        Layout.fillWidth: true
        allLanguages: languageUtils.languages
        usedLanguages: languageUtils.usedLanguages
        checkedLangCodes: userSettings.getContentLanguages(userDid)
        onCheckedLangCodesChanged: userSettings.setContentLanguages(userDid, checkedLangCodes)
    }

    AccessibleCheckBox {
        text: qsTr("Show posts without language tag")
        checked: userSettings.getShowUnknownContentLanguage(userDid)
        onCheckedChanged: userSettings.setShowUnknownContentLanguage(userDid, checked)
    }

    AccessibleCheckBox {
        text: qsTr("Show language tags on post")
        checked: userSettings.getShowLanguageTags()
        onCheckedChanged: userSettings.setShowLanguageTags(checked)
    }

    AccessibleCheckBox {
        text: qsTr("Translate link for foreign post language")
        checked: userSettings.showTranslateLink
        onCheckedChanged: userSettings.showTranslateLink = checked
    }

    AccessibleText {
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        text: qsTr("Post language detection:")
    }
    SkyComboBox {
        Layout.fillWidth: true
        model: ListModel {
            ListElement { value: QEnums.LANGUAGE_DETECTION_TAG; text: qsTr("Language tag from post") }
            ListElement { value: QEnums.LANGUAGE_DETECTION_AUTO; text: qsTr("Guess from text") }
            ListElement { value: QEnums.LANGUAGE_DETECTION_TAG_AUTO; text: qsTr("Guess when tag is missing") }
        }
        currentIndex: userSettings.getLanguageDetectionMethod()
        onCurrentValueChanged: userSettings.setLanguageDetectionMethod(currentValue)
    }

    AccessibleText {
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        text: qsTr("Exclude languages for translate link:")
    }
    LanguageComboCheckBox {
        Layout.fillWidth: true
        allLanguages: languageUtils.languages
        usedLanguages: languageUtils.usedLanguages
        checkedLangCodes: userSettings.getExcludeTranslateLanguages(userDid)
        noneCheckedMeansAll: false
        onCheckedLangCodesChanged: userSettings.setExcludeTranslateLanguages(userDid, checkedLangCodes)
    }

    AccessibleText {
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        text: qsTr("App for translation:")
    }
    SkyComboBox {
        Layout.fillWidth: true
        model: ListModel {
            ListElement { value: QEnums.TRANSLATE_APP_GOOGLE; text: qsTr("Google Translate") }
            ListElement { value: QEnums.TRANSLATE_APP_DEEPL; text: qsTr("DeepL") }
            ListElement { value: QEnums.TRANSLATE_APP_OTHER; text: qsTr("Other") }
        }
        currentIndex: userSettings.getTranslateApp()
        onCurrentValueChanged: userSettings.setTranslateApp(currentValue)
    }

    AccessibleText {
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        text: qsTr("Exclude languages from auto detection for post writing:")
    }
    LanguageComboCheckBox {
        Layout.fillWidth: true
        allLanguages: languageUtils.languages
        usedLanguages: languageUtils.usedLanguages
        checkedLangCodes: userSettings.getExcludeDetectLanguages(userDid)
        noneCheckedMeansAll: false
        onCheckedLangCodesChanged: userSettings.setExcludeDetectLanguages(userDid, checkedLangCodes)
    }

    LanguageUtils {
        id: languageUtils
        skywalker: column.skywalker
    }
}
