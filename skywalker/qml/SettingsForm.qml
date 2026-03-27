import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

SkyPage {
    property Skywalker skywalker: root.getSkywalker()
    property var userPrefs: skywalker.getEditUserPreferences()
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
        onBack: closed()

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

            SettingsChat {
                Layout.leftMargin: 10
                Layout.rightMargin: 10
                Layout.fillWidth: true
                userPrefs: page.userPrefs
                visible: allVisible || onlyChatVisible
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
        property var prefs

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

    Component.onDestruction: {
        console.debug("Save settings")
        skywalker.saveUserPreferences()

        if (notificationsLoader.active)
            notificationtUtils.saveNotificationPrefs()
    }

    Component.onCompleted: {
        if (allVisible || onlyNotificationVisible)
            notificationtUtils.getNotificationPrefs()
    }
}
