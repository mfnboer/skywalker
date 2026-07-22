import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

SkyPage {
    property Skywalker skywalker: root.getSkywalker()
    property EditUserPreferences userPrefs: skywalker.getEditUserPreferences()
    property bool allVisible: true
    property bool onlyChatVisible: false
    property bool onlyNotificationVisible: false
    readonly property string sideBarTitle: qsTr("Settings")
    readonly property SvgImage sideBarSvg: SvgOutline.settings

    id: page
    topPadding: 10
    bottomPadding: 10

    signal closed()

    Accessible.role: Accessible.Pane

    header: SimpleHeader {
        property int prevHeight: -1

        text: sideBarTitle
        visible: !root.showSideBar
        onBack: saveAndClose()

        onHeightChanged: {
            if (prevHeight < 0)
                return

            const dh = height - prevHeight
            flick.contentY += dh
            prevHeight = height
        }

        function initPrevHeight() {
            prevHeight = height
        }
    }

    Flickable {
        id: flick
        anchors.fill: parent
        clip: true
        contentWidth: parent.width
        contentHeight: contentItem.childrenRect.height
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds

        ColumnLayout {
            width: parent.width

            SettingsAccount {
                Layout.leftMargin: 10
                Layout.rightMargin: 10
                Layout.fillWidth: true
                userPrefs: page.userPrefs
                visible: allVisible
            }

            Rectangle {
                Layout.topMargin: 10
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: guiSettings.separatorColor
                visible: allVisible
            }

            SettingsSecurity {
                Layout.leftMargin: 10
                Layout.rightMargin: 10
                Layout.fillWidth: true
                userPrefs: page.userPrefs
                visible: allVisible
            }

            Rectangle {
                Layout.topMargin: 10
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: guiSettings.separatorColor
                visible: allVisible
            }

            SettingsHomeFeed {
                Layout.leftMargin: 10
                Layout.rightMargin: 10
                Layout.fillWidth: true
                userPrefs: page.userPrefs
                visible: allVisible
            }

            Rectangle {
                Layout.topMargin: 10
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: guiSettings.separatorColor
                visible: allVisible
            }

            SettingsGeneralFeed {
                Layout.leftMargin: 10
                Layout.rightMargin: 10
                Layout.fillWidth: true
                visible: allVisible
            }

            Rectangle {
                Layout.topMargin: 10
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: guiSettings.separatorColor
                visible: allVisible
            }

            Loader {
                id: chatLoader
                Layout.leftMargin: 10
                Layout.rightMargin: 10
                Layout.fillWidth: true
            }

            Rectangle {
                Layout.topMargin: 10
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: guiSettings.separatorColor
                visible: allVisible
            }

            SettingsSearch {
                Layout.leftMargin: 10
                Layout.rightMargin: 10
                Layout.fillWidth: true
                visible: allVisible
            }

            Rectangle {
                Layout.topMargin: 10
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: guiSettings.separatorColor
                visible: allVisible
            }

            SettingsLanguage {
                Layout.leftMargin: 10
                Layout.rightMargin: 10
                Layout.fillWidth: true
                visible: allVisible
            }

            Rectangle {
                Layout.topMargin: 10
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: guiSettings.separatorColor
                visible: allVisible
            }

            SettingsAppearance {
                Layout.leftMargin: 10
                Layout.rightMargin: 10
                Layout.fillWidth: true
                userPrefs: page.userPrefs
                visible: allVisible

                onOffsetY: (dy) => flick.contentY += dy
                onFontScaleChanged: page.header.initPrevHeight()
            }

            Rectangle {
                Layout.topMargin: 10
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: guiSettings.separatorColor
                visible: allVisible
            }

            SettingsExternalContent {
                Layout.leftMargin: 10
                Layout.rightMargin: 10
                Layout.fillWidth: true
                visible: allVisible
            }

            Rectangle {
                Layout.topMargin: 10
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: guiSettings.separatorColor
                visible: allVisible
            }

            Loader {
                id: notificationsLoader
                Layout.leftMargin: 10
                Layout.rightMargin: 10
                Layout.fillWidth: true
            }
        }
    }

    Component {
        id: notificationComponent

        SettingsNotifications {
            id: settingsNotifications
            notificationPrefs: notificationtUtils.prefs
        }
    }

    NotificationUtils {
        property EditNotificationPreferences prefs

        id: notificationtUtils
        skywalker: page.skywalker

        onNotificationPrefsOk: (prefs) => {
            notificationtUtils.prefs = prefs
            notificationsLoader.sourceComponent = notificationComponent
            notificationsLoader.active = true
        }

        onNotificationPrefsFailed: (error) => {
            skywalker.showStatusMessage(qsTr(`Cannot get notification preferences: ${error}`), QEnums.STATUS_LEVEL_ERROR)
        }
    }

    Component {
        id: chatComponent

        SettingsChat {
            id: settingsChat
            userPrefs: page.userPrefs
            chatNotificationPrefs: chatNotificationUtils.prefs
        }
    }

    ChatNotificationUtils {
        property EditChatNotificationPreferences prefs

        id: chatNotificationUtils
        skywalker: page.skywalker

        onChatNotificationPrefsOk: (prefs) => {
            chatNotificationUtils.prefs = prefs
            chatLoader.sourceComponent = chatComponent
            chatLoader.active = true
        }

        onChatNotificationPrefsFailed: (error) => {
            skywalker.showStatusMessage(qsTr(`Cannot get chat notification preferences: ${error}`), QEnums.STATUS_LEVEL_ERROR)
        }
    }

    function saveAndClose() {
        console.debug("Save settings")
        skywalker.saveUserPreferences()

        if (notificationsLoader.active)
            notificationtUtils.saveNotificationPrefs()

        if (chatLoader.active)
            chatNotificationUtils.saveChatNotificationPrefs()

        closed()
    }

    // The Android back-button should also save all changes
    function cancel() {
        saveAndClose()
    }

    Component.onCompleted: {
        if (allVisible || onlyNotificationVisible)
            notificationtUtils.getNotificationPrefs()

        if (allVisible || onlyChatVisible)
            chatNotificationUtils.getChatNotificationPrefs()
    }
}
