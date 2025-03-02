import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

SkyPage {
    property var skywalker: root.getSkywalker()
    property var userPrefs: skywalker.getEditUserPreferences()
    property bool allVisible: true
    property bool onlyChatVisible: false

    id: page
    padding: 10

    signal closed()

    Accessible.role: Accessible.Pane

    header: Rectangle {
        width: parent.width
        height: guiSettings.headerHeight
        z: guiSettings.headerZLevel
        color: guiSettings.headerColor

        RowLayout
        {
            id: headerRow

            SvgPlainButton {
                id: backButton
                iconColor: guiSettings.headerTextColor
                svg: SvgOutline.arrowBack
                accessibleName: qsTr("go back")
                onClicked: page.closed()
            }
            AccessibleText {
                id: headerTexts
                Layout.alignment: Qt.AlignVCenter
                font.bold: true
                font.pointSize: guiSettings.scaledFont(10/8)
                color: guiSettings.headerTextColor
                text: qsTr("Settings")
            }
        }
    }

    Flickable {
        anchors.fill: parent
        clip: true
        contentWidth: parent.width
        contentHeight: contentItem.childrenRect.height
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds

        SettingsAccount {
            id: settingsAccount
            width: parent.width
            height: visible ? undefined : 0
            userPrefs: page.userPrefs
            visible: allVisible
        }

        SettingsHomeFeed {
            id: settingsHomeFeed
            anchors.top: settingsAccount.bottom
            width: parent.width
            height: visible ? undefined : 0
            userPrefs: page.userPrefs
            visible: allVisible
        }

        SettingsChat {
            id: settingsChat
            anchors.top: settingsHomeFeed.bottom
            width: parent.width
            height: visible ? undefined : 0
            userPrefs: page.userPrefs
            visible: allVisible || onlyChatVisible
        }

        SettingsSearch {
            id: settingsSearch
            anchors.top: settingsChat.bottom
            width: parent.width
            height: visible ? undefined : 0
            visible: allVisible
        }

        SettingsLanguage {
            id: settingsLanguage
            anchors.top: settingsSearch.bottom
            width: parent.width
            height: visible ? undefined : 0
            visible: allVisible
        }

        SettingsAppearance {
            id: settingsAppearance
            anchors.top: settingsLanguage.bottom
            width: parent.width
            height: visible ? undefined : 0
            visible: allVisible
        }

        SettingsNotifications {
            id: settingsNotifications
            anchors.top: settingsAppearance.bottom
            width: parent.width
            height: visible ? undefined : 0
            visible: allVisible
        }
    }

    Component.onDestruction: {
        console.debug("Save settings");
        skywalker.saveUserPreferences();
    }
}
