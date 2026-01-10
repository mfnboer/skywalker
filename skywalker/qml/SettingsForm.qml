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
    property bool onlyPostThreadVisible: false
    readonly property string sideBarTitle: qsTr("Settings")
    readonly property SvgImage sideBarSvg: SvgOutline.settings

    id: page
    padding: 10

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
            userPrefs: page.userPrefs
            visible: allVisible

            onOffsetY: (dy) => flick.contentY += dy
            onFontScaleChanged: page.header.initPrevHeight()
        }

        Loader {
            id: notificationsLoader
            anchors.top: settingsAppearance.bottom
            width: parent.width
        }

        SettingsPostThread {
            id: settingsPostThread
            anchors.top: notificationsLoader.bottom
            width: parent.width
            height: visible ? undefined : 0
            visible: allVisible || onlyPostThreadVisible
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
