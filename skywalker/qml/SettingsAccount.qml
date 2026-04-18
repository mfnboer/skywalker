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
                onClicked: skywalker.getShareUtils().copyToClipboard(userPrefs.did)
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
            text: qsTr("Login:")
        }
        AccessibleText {
            Layout.fillWidth: true
            elide: Text.ElideRight
            text: userSettings.getOAuthEnabled(userPrefs.did) ? "OAuth" : "password"
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

    function confirmEmail() {
        guiSettings.askYesNoQuestion(root,
            qsTr(`Do you want to confirm your email address <b>${email}</b>? You will receive a code in email to do so.<br><br>` +
                 `<a href="settings" style="color: ${guiSettings.linkColor}; text-decoration: none">I already have a code</a>.`),
            () => { accountUtils.requestEmailConfirmation() },
            () => {},
            (link) => { accountUtils.enterEmailConfirmationToken() }
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


        function enterEmailConfirmationToken() {
            guiSettings.askToken(root, "Email confirmation code",
                (token) => { accountUtils.confirmEmail(token) }
            )
        }
    }

    function changeEmail() {
        if (userSettings.getOAuthEnabled(userPrefs.did)) {
            skywalker.showStatusMessage(qsTr("Email address cannot be changed when you are logged in with OAuth"), QEnums.STATUS_LEVEL_INFO, 5)
            return
        }

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
        dialog.onEmailToken.connect((email, token) => { dialog.close(); onOkCb(email, token) })
        dialog.onRejected.connect(() => { dialog.close() })
        dialog.open()
    }
}
