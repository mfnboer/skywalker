import QtQuick
import QtQuick.Controls
import skywalker

SkyMenuButton {
    property Skywalker skywalker: root.getSkywalker()
    property UserSettings userSettings: skywalker.getUserSettings()

    id: translateButton
    text: qsTr("Translate")
    svg: getIcon()

    SvgPlainButton {
        anchors.right: parent.right
        anchors.rightMargin: 5
        anchors.verticalCenter: parent.verticalCenter
        svg: SvgOutline.settings
        accessibleName: qsTr("choose translation app")
        onClicked: appMenu.open()
    }

    SkyPopup {
        id: appMenu
        y: translateButton.height + 5
        width: parent.width
        height: contentHeight + topPadding + bottomPadding
        padding: 5
        modal: true

        background: Rectangle {
            radius: 5
            color: guiSettings.backgroundColor
        }

        Column {
            width: parent.width

            SkyMenuButton {
                text: qsTr("Google Translate")
                svg: SvgOutline.googleTranslate
                popup: appMenu
                onClicked: {
                    userSettings.setTranslateApp(QEnums.TRANSLATE_APP_GOOGLE)
                    translateButton.click()
                }
            }
            SkyMenuButton {
                text: qsTr("DeepL")
                svg: SvgOutline.deepLIcon
                popup: appMenu
                onClicked: {
                    userSettings.setTranslateApp(QEnums.TRANSLATE_APP_DEEPL)
                    translateButton.click()
                }
            }
            SkyMenuButton {
                text: qsTr("Other")
                svg: SvgOutline.translate
                popup: appMenu
                onClicked: {
                    userSettings.setTranslateApp(QEnums.TRANSLATE_APP_OTHER)
                    translateButton.click()
                }
            }
        }
    }

    function click() {
        clicked()

        if (popup)
            popup.close()
    }

    function getIcon() {
        switch (userSettings.getTranslateApp()) {
        case QEnums.TRANSLATE_APP_GOOGLE:
            return SvgOutline.googleTranslate
        case QEnums.TRANSLATE_APP_DEEPL:
            return SvgOutline.deepLIcon
        default:
            return SvgOutline.translate
        }
    }
}

