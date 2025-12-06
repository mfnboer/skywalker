import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

GridLayout {
    required property var userPrefs
    property var skywalker: root.getSkywalker()

    columns: 2

    HeaderText {
        Layout.columnSpan: 2
        text: qsTr("Account")
    }

    AccessibleText {
        text: qsTr("Email:")
    }
    RowLayout {
        Layout.fillWidth: true

        AccessibleText {
            id: mailText
            Layout.fillWidth: true
            elide: Text.ElideRight
            text: userPrefs.email
        }
        SvgPlainButton {
            id: mailConfirmedImg
            imageMargin: 0
            implicitWidth: height
            implicitHeight: mailText.height
            iconColor: guiSettings.buttonColor
            accessibleName: qsTr("E-mail address confirmed")
            svg: SvgOutline.check
            visible: userPrefs.emailConfirmed
            onClicked: skywalker.showStatusMessage(accessibleName, QEnums.STATUS_LEVEL_INFO)
        }
        SvgPlainButton {
            imageMargin: 0
            implicitWidth: height
            implicitHeight: mailText.height
            accessibleName: qsTr("Two-factor authentication enabled")
            svg: SvgOutline.confirmationCode
            visible: userPrefs.emailAuthFactor
            onClicked: skywalker.showStatusMessage(accessibleName, QEnums.STATUS_LEVEL_INFO)
        }
    }

    AccessibleText {
        color: guiSettings.textColor
        text: qsTr("Birthday:")
    }
    AccessibleText {
        Layout.fillWidth: true
        color: guiSettings.textColor
        text: userPrefs.birthDate
    }

    AccessibleText {
        color: guiSettings.textColor
        text: "PDS:"
    }
    AccessibleText {
        Layout.fillWidth: true
        color: guiSettings.textColor
        elide: Text.ElideRight
        text: userPrefs.pds
    }

    AccessibleText {
        id: didLabel
        color: guiSettings.textColor
        text: "DID:"
    }
    RowLayout {
        Layout.fillWidth: true

        AccessibleText {
            Layout.fillWidth: true
            color: guiSettings.textColor
            elide: Text.ElideRight
            text: userPrefs.did
        }
        SvgPlainButton {
            imageMargin: 0
            implicitWidth: height
            implicitHeight: didLabel.height
            svg: SvgOutline.copy
            accessibleName: qsTr("copy") + " D I D"
            onClicked: skywalker.copyToClipboard(userPrefs.did)
        }
    }

    Rectangle {
        Layout.columnSpan: 2
        Layout.fillWidth: true
        Layout.preferredHeight: advancedButton.height
        color: "transparent"

        SkyButton {
            id: advancedButton
            implicitHeight: 40
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Advanced settings")
            onClicked: root.editAdvancedSettings()
        }
    }

    HeaderText {
        Layout.columnSpan: 2
        Layout.topMargin: 10
        text: qsTr("Logged-out visibility")
    }

    AccessibleCheckBox {
        id: loggedoutSwitch
        Layout.columnSpan: 2
        Layout.fillWidth: true
        text: qsTr("Discourage apps from showing my account to logged-out users")

        checked: !userPrefs.loggedOutVisibility
        onCheckedChanged: userPrefs.loggedOutVisibility = !checked
    }
}
