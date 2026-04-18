import QtQuick
import QtQuick.Controls
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

    ColumnLayout {
        id: grid
        width: parent.width

        HeaderText {
            Layout.topMargin: 10
            text: qsTr("Privacy and security")
        }

        AccessibleCheckBox {
            Layout.fillWidth: true
            text: qsTr("Remember password")
            checked: userSettings.getRememberPassword(userPrefs.did)
            visible: !userSettings.getOAuthEnabled(userPrefs.did)
            onCheckedChanged: userSettings.setRememberPassword(userPrefs.did, checked)
        }

        AccessibleCheckBox {
            id: emailAuthFactorSwitch
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
            Layout.fillWidth: true
            text: qsTr("Discourage apps from showing my account to logged-out users")

            checked: !userPrefs.loggedOutVisibility
            onCheckedChanged: userPrefs.loggedOutVisibility = !checked
        }

        AccessibleText {
            Layout.topMargin: 10
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            textFormat: Text.RichText
            text: qsTr(`<a href="settings" style="color: ${guiSettings.linkColor}; text-decoration: none">Change password</a>`)
            onLinkActivated: changePassword()
        }
    }

    function update2FA() {
        if (userSettings.getOAuthEnabled(userPrefs.did)) {
            skywalker.showStatusMessage(qsTr("Email settings cannot be changed when you are logged in with OAuth"), QEnums.STATUS_LEVEL_INFO, 5)
            emailAuthFactorSwitch.checked = emailAuthFactor
            return
        }

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

        onResetPasswordOk: (password) => {
            if (userSettings.getRememberPassword(userPrefs.did))
                userSettings.savePassword(userPrefs.did, password)

            skywalker.showStatusMessage(qsTr("Password changed"), QEnums.STATUS_LEVEL_INFO)
        }

        onResetPasswordFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)

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

    BusyIndicator {
        anchors.centerIn: parent
        running: accountUtils.updateInProgress
    }
}
