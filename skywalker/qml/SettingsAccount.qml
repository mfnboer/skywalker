import QtQuick
import QtQuick.Layouts
import skywalker

Item {
    required property var userPrefs
    property bool emailAuthFactor: userPrefs.emailAuthFactor
    property bool emailConfirmed: userPrefs.emailConfirmed
    property string email: userPrefs.email
    property Skywalker skywalker: root.getSkywalker()
    property UserSettings userSettings: skywalker.getUserSettings()

    id: section
    Layout.preferredHeight: visible ? grid.height : 0

    GridLayout {
        id: grid
        width: parent.width
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
                text: email
            }
            SvgPlainButton {
                id: mailConfirmedImg
                imageMargin: 0
                implicitWidth: height
                implicitHeight: mailText.height
                iconColor: guiSettings.buttonColor
                accessibleName: qsTr("E-mail address confirmed")
                svg: SvgOutline.check
                visible: emailConfirmed
                onClicked: skywalker.showStatusMessage(accessibleName, QEnums.STATUS_LEVEL_INFO)
            }
            SvgPlainButton {
                imageMargin: 0
                implicitWidth: height
                implicitHeight: mailText.height
                accessibleName: qsTr("Two-factor authentication enabled")
                svg: SvgOutline.confirmationCode
                visible: emailAuthFactor
                onClicked: skywalker.showStatusMessage(accessibleName, QEnums.STATUS_LEVEL_INFO)
            }
        }

        // Birthday is not given when using OAuth
        AccessibleText {
            text: qsTr("Birthday:")
            visible: Boolean(userPrefs.birthDate)
        }
        AccessibleText {
            Layout.fillWidth: true
            text: userPrefs.birthDate
            visible: Boolean(userPrefs.birthDate)
        }

        AccessibleText {
            text: "PDS:"
        }
        AccessibleText {
            Layout.fillWidth: true
            elide: Text.ElideRight
            text: userPrefs.pds
        }

        AccessibleText {
            id: didLabel
            text: "DID:"
        }
        RowLayout {
            Layout.fillWidth: true

            AccessibleText {
                Layout.fillWidth: true
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

        AccessibleText {
            text: qsTr("Provider:")
        }
        AccessibleText {
            Layout.fillWidth: true
            elide: Text.ElideRight
            text: userSettings.getHost(userPrefs.did)
        }

        AccessibleCheckBox {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            text: qsTr("Add automation label to account to show the world that this account is automated. 🤖 is shown in front of the name.")

            checked: userPrefs.automatedAccount
            onCheckedChanged: userPrefs.automatedAccount = checked
        }

        AccessibleText {
            Layout.columnSpan: 2
            Layout.topMargin: 10
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            textFormat: Text.RichText
            text: qsTr(`<a href="settings" style="color: ${guiSettings.linkColor}; text-decoration: none">Confirm email address</a>`)
            visible: !emailConfirmed
            onLinkActivated: confirmEmail()
        }

        AccessibleText {
            Layout.columnSpan: 2
            Layout.topMargin: 10
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            textFormat: Text.RichText
            text: qsTr(`<a href="settings" style="color: ${guiSettings.linkColor}; text-decoration: none">Change email address</a>`)
            onLinkActivated: changeEmail()
        }

        AccessibleText {
            Layout.columnSpan: 2
            Layout.topMargin: 10
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            textFormat: Text.RichText
            text: qsTr(`<a href="settings" style="color: ${guiSettings.linkColor}; text-decoration: none">Advanced settings</a>`)
            onLinkActivated: root.editAdvancedSettings()
        }
    }
}
