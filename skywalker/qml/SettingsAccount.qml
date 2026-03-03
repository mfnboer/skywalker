import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

Item {
    required property var userPrefs
    property bool emailAuthFactor: userPrefs.emailAuthFactor
    property Skywalker skywalker: root.getSkywalker()
    property UserSettings userSettings: skywalker.getUserSettings()

    id: section
    height: visible ? grid.height : 0

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
                visible: emailAuthFactor
                onClicked: skywalker.showStatusMessage(accessibleName, QEnums.STATUS_LEVEL_INFO)
            }
        }

        AccessibleText {
            text: qsTr("Birthday:")
        }
        AccessibleText {
            Layout.fillWidth: true
            text: userPrefs.birthDate
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
            text: qsTr("Privacy and security")
        }

        AccessibleCheckBox {
            id: emailAuthFactorSwitch
            Layout.columnSpan: 2
            Layout.fillWidth: true
            text: qsTr("Two-factor authentication (2FA)")

            checked: emailAuthFactor
            onCheckedChanged: {
                if (checked !== emailAuthFactor)
                    update2FA()
            }
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

    function update2FA() {
        if (emailAuthFactor) {
            guiSettings.askYesNoQuestion(root,
                qsTr("Do you want to disable 2FA? You will receive a code in email to do so.<br><br>" +
                     `<a href="settings" style="color: ${guiSettings.linkColor}; text-decoration: none">I already have a token</a>.`),
                () => { accountUtils.requestEmailUpdateToken() },
                () => { emailAuthFactorSwitch.checked = true },
                (link) => { accountUtils.enterToken() }
            )
        } else {
            guiSettings.noticeOkCancel(root,
                qsTr("When you sign in to your account you will receive a login code in email."),
                () => { accountUtils.update2FA(true, "") },
                () => { emailAuthFactorSwitch.checked = false }
            )
        }
    }

    AccountUtils {
        id: accountUtils
        skywalker: section.skywalker

        onUpdate2FAOk: (enabled) => {
            if (enabled) {
                skywalker.showStatusMessage(qsTr("2FA enabled"), QEnums.STATUS_LEVEL_INFO)
                emailAuthFactor = true
            } else {
                skywalker.showStatusMessage(qsTr("2FA disabled"), QEnums.STATUS_LEVEL_INFO)
                emailAuthFactor = false
            }
        }

        onUpdate2FAFailed: (error) => {
            skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
            emailAuthFactorSwitch.checked = emailAuthFactor
        }

        onEmailUpdateTokenOk: enterToken()

        onEmailUpdateTokenNotRequired: update2FA(false, "")

        onEmailUpdateTokenFailed: (error) => {
            skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
            emailAuthFactorSwitch.checked = emailAuthFactor
        }

        function enterToken() {
            guiSettings.askToken(root, qsTr("Email update token"),
                (token) => { accountUtils.update2FA(false, token) },
                () => { emailAuthFactorSwitch.checked = true }
            )
        }
    }

    BusyIndicator {
        anchors.centerIn: parent
        running: accountUtils.emailUpdateInProgress
    }

}
