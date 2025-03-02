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
        color: guiSettings.textColor
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

    LanguageUtils {
        id: languageUtils
        skywalker: column.skywalker
    }
}
