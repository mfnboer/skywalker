import QtQuick
import skywalker

SvgPlainButton {
    property list<language> filteredLanguages
    property bool showPostWithMissingLanguage: true

    svg: SvgOutline.language
    iconColor: guiSettings.headerTextColor
    accessibleName: qsTr("language filter active")
    onClicked: showLanguageFilterDetails()

    function showLanguageFilterDetails() {
        let languageList = []

        for (let i = 0; i < filteredLanguages.length; ++i) {
            const lang = `${filteredLanguages[i].nativeName} (${filteredLanguages[i].shortCode})`
            languageList.push(lang)
            console.debug(lang)
        }

        let msg = qsTr("Language filter is active.") + "<br><br>"

        if (languageList.length > 0) {
            msg += qsTr("Only posts with the following languages will be shown:") +
                    " " + languageList.join(", ") + "<br><br>"
        }

        if (!showPostWithMissingLanguage)
            msg += qsTr("Posts without language tags will not be shown.")
        else
            msg += qsTr("Posts without language tags will be shown.")

        guiSettings.notice(rootContent, msg)
    }
}
