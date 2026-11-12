import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyPage {
    property Skywalker skywalker: root.getSkywalker()
    property bool preferencesLoaded: false
    readonly property string sideBarTitle: qsTr("Moderation inbox settings")
    readonly property SvgImage sideBarSvg: SvgOutline.settings

    id: page
    topPadding: 10
    bottomPadding: 10

    signal closed()

    Accessible.role: Accessible.Pane

    header: SimpleHeader {
        text: sideBarTitle
        visible: !root.showSideBar
        onBack: closed()
    }

    Flickable {
        id: flick
        anchors.fill: parent
        clip: true
        contentWidth: parent.width
        contentHeight: settingsCol.height
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds

        ColumnLayout {
            id: settingsCol
            x: 10
            width: parent.width - 20

            AccessibleCheckBox {
                id: reportsCheckBox
                Layout.fillWidth: true
                text: qsTr("Notifications about your reports")
                enabled: preferencesLoaded && !ozoneUtils.notificationPreferencesInProgress
                onCheckedChanged: ozoneUtils.save(reportsCheckBox)
            }
            AccessibleCheckBox {
                id: accountCheckBox
                Layout.fillWidth: true
                text: qsTr("Notifications about actions taken against you")
                enabled: preferencesLoaded && !ozoneUtils.notificationPreferencesInProgress
                onCheckedChanged: ozoneUtils.save(accountCheckBox)
            }
            AccessibleCheckBox {
                id: labelsCheckBox
                Layout.fillWidth: true
                text: qsTr("Notifications when a labels is added to your account or posts")
                enabled: preferencesLoaded && !ozoneUtils.notificationPreferencesInProgress
                onCheckedChanged: ozoneUtils.save(labelsCheckBox)
            }
        }
    }

    BusyIndicator {
        anchors.centerIn: parent
        running: ozoneUtils.notificationPreferencesInProgress
    }

    OzoneUtils {
        property var savingCheckBox

        id: ozoneUtils
        skywalker: page.skywalker

        onGetNotificationPreferencesOk: (reports, account, labels) => {
            reportsCheckBox.checked = reports
            accountCheckBox.checked = account
            labelsCheckBox.checked = labels
            page.preferencesLoaded = true
        }

        onGetNotificationPreferencesFailed: (error) => {
            skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
            page.closed()
        }

        onPutNotificationPreferencesOk: savingCheckBox = null

        onPutNotificationPreferencesFailed: (error) => {
            savingCheckBox.checked = !savingCheckBox.checked
            savingCheckBox = null
            skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
        }

        function save(checkBox) {
            // Prevent saving loop on value restoration after failure
            if (savingCheckBox || !page.preferencesLoaded)
                return

            console.debug("Save settings")
            savingCheckBox = checkBox
            putNotificationPreferences(reportsCheckBox.checked,
                                       accountCheckBox.checked,
                                       labelsCheckBox.checked)
        }
    }

    Component.onCompleted: {
        ozoneUtils.getNotificationPreferences()
    }
}
