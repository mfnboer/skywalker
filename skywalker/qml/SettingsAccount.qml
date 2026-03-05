import QtQuick
import QtQuick.Controls.Material
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
            visible: emailConfirmed

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

        AccessibleText {
            Layout.columnSpan: 2
            Layout.topMargin: 10
            Layout.bottomMargin: 10
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            textFormat: Text.RichText
            text: qsTr(`<a href="settings" style="color: ${guiSettings.linkColor}; text-decoration: none">Change password</a>`)
            onLinkActivated: changePassword()
        }
    }

    function confirmEmail() {
        guiSettings.askYesNoQuestion(root,
            qsTr(`Do you want to confirm your email address <b>${email}</b>? You will receive a code in email to do so.<br><br>` +
                 `<a href="settings" style="color: ${guiSettings.linkColor}; text-decoration: none">I already have a code</a>.`),
            () => { accountUtils.requestEmailConfirmation() },
            () => {},
            (link) => { accountUtils.enterEmailConfirmationToken() }
        )
    }

    function update2FA() {
        if (emailAuthFactor) {
            guiSettings.askYesNoQuestion(root,
                qsTr("Do you want to disable 2FA? You will receive a code in email to do so.<br><br>" +
                     `<a href="settings" style="color: ${guiSettings.linkColor}; text-decoration: none">I already have a code</a>.`),
                () => { accountUtils.requestEmailUpdateToken() },
                () => { emailAuthFactorSwitch.checked = true },
                (link) => { accountUtils.enterEmailUpdateToken() }
            )
        } else {
            guiSettings.noticeOkCancel(root,
                qsTr("When you sign in to your account you will receive a login code in email."),
                () => { accountUtils.update2FA(true, "") },
                () => { emailAuthFactorSwitch.checked = false }
            )
        }
    }

    function changePassword() {
        guiSettings.askYesNoQuestion(root,
            qsTr("Do you want to change your password? We will send you a code to verify that this is your account.<br><br>" +
                 `<a href="settings" style="color: ${guiSettings.linkColor}; text-decoration: none">I already have a code</a>.`),
            () => { accountUtils.requestPasswordReset() },
            () => {},
            (link) => { accountUtils.enterPasswordResetToken() }
        )
    }

    AccountUtils {
        id: accountUtils
        skywalker: section.skywalker

        onConfirmEmailOk: {
            skywalker.showStatusMessage(qsTr("Email address confirmed"), QEnums.STATUS_LEVEL_INFO)
            emailConfirmed = true
        }
        onConfirmEmailFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)

        onRequestEmailConfirmationOk: enterEmailConfirmationToken()
        onRequestEmailConfirmationFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)

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

        onEmailUpdateTokenOk: enterEmailUpdateToken()

        onEmailUpdateTokenNotRequired: update2FA(false, "")

        onEmailUpdateTokenFailed: (error) => {
            skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
            emailAuthFactorSwitch.checked = emailAuthFactor
        }

        onRequestResetPasswordOk: enterPasswordResetToken()
        onRequestResetPasswordFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
        onResetPasswordOk: skywalker.showStatusMessage(qsTr("Password changed"), QEnums.STATUS_LEVEL_INFO)
        onResetPasswordFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)

        function enterEmailConfirmationToken() {
            guiSettings.askToken(root, "Email confirmation code",
                (token) => { accountUtils.confirmEmail(token) }
            )
        }

        function enterEmailUpdateToken() {
            guiSettings.askToken(root, "Email update (2FA) code",
                (token) => { accountUtils.update2FA(false, token) },
                () => { emailAuthFactorSwitch.checked = true }
            )
        }

        function enterPasswordResetToken() {
            guiSettings.askPasswordResetToken(root,
                (password, token) => { accountUtils.resetPassword(password, token) }
            )
        }
    }

    function changeEmail() {
        guiSettings.askYesNoQuestion(root,
            qsTr(`Do you want to change your email address <b>${email}</b>?` +
                 (emailConfirmed ?
                    (" You will receive a code in email to do so.<br><br>" +
                     `<a href="settings" style="color: ${guiSettings.linkColor}; text-decoration: none">I already have a code</a>.`) :
                    "")),
            () => { emailUpdater.requestEmailUpdateToken() },
            () => {},
            (link) => { emailUpdater.enterEmailUpdateToken() }
        )
    }

    AccountUtils {
        id: emailUpdater
        skywalker: section.skywalker

        onUpdateEmailOk: (newEmail) => {
            skywalker.showStatusMessage(qsTr("Email address updated"), QEnums.STATUS_LEVEL_INFO)
            email = newEmail
            emailAuthFactor = false
            emailConfirmed = false
        }

        onUpdateEmailFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)

        onEmailUpdateTokenOk: enterEmailUpdateToken(true)
        onEmailUpdateTokenNotRequired: enterEmailUpdateToken(false)
        onEmailUpdateTokenFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)

        function enterEmailUpdateToken(tokenRequired) {
            askEmailUpdateToken(tokenRequired,
                (email, token) => { emailUpdater.updateEmail(email, token) }
            )
        }
    }

    BusyIndicator {
        anchors.centerIn: parent
        running: accountUtils.updateInProgress || emailUpdater.updateInProgress
    }

    function askEmailUpdateToken(tokenRequired, onOkCb) {
        let component = guiSettings.createComponent("TokenEmailDialog.qml")
        let dialog = component.createObject(root, { tokenRequired: tokenRequired })
        dialog.onEmailToken.connect((email, token) => { dialog.destroy(); onOkCb(email, token) })
        dialog.onRejected.connect(() => { dialog.destroy() })
        dialog.open()
    }
}
