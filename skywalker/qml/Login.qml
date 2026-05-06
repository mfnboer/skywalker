import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker
import atproto.lib

SkyPage {
    property string did
    property string host
    property string user
    property string errorCode
    property string errorMsg
    property string password
    property bool useOAuth: false
    property bool setAdvancedSettings: false
    property string serviceAppView
    property string serviceChat
    property string serviceVideoHost
    property string serviceVideoDid
    property Skywalker skywalker: root.getSkywalker()
    property UserSettings userSettings: skywalker.getUserSettings()
    readonly property string sideBarTitle: isNewAccount() ? qsTr("Add Account") : qsTr("Login")

    signal accepted(bool useOAuth,
                    string host,
                    string handle,
                    string password,
                    string did,
                    bool rememberPassword,
                    string authFactorTokenField,
                    bool setAdvancedSettings,
                    string serviceAppView,
                    string serviceChat,
                    string serviceVideoHost,
                    string serviceVideoDid)
    signal canceled

    id: loginPage
    width: parent.width
    padding: 10

    Accessible.role: Accessible.Pane

    header: SimpleHeader {
        text: sideBarTitle
        visible: !root.showSideBar
        onBack: loginPage.canceled()
    }

    footer: Item {
        width: loginPage.width
        height: keyboardHandler.keyboardVisible ? keyboardHandler.keyboardHeight : 0
    }

    SkyTabBar {
        id: authBar
        width: parent.width

        AccessibleTabButton {
            text: qsTr("App password")
        }
        AccessibleTabButton {
            text: qsTr("Web (OAuth)")
        }
    }

    SwipeView {
        anchors.top: authBar.bottom
        anchors.bottom: parent.bottom
        width: parent.width
        currentIndex: authBar.currentIndex

        onCurrentIndexChanged: authBar.setCurrentIndex(currentIndex)

        Flickable {
            id: flick
            contentHeight: okButton.y + okButton.height
            flickableDirection: Flickable.VerticalFlick
            boundsBehavior: Flickable.StopAtBounds

            ColumnLayout {
                id: loginForm
                width: parent.width
                Accessible.role: Accessible.Pane

                AccessibleText {
                    Layout.fillWidth: true
                    topPadding: 10
                    leftPadding: 10
                    font.bold: true
                    text: qsTr("Hosting provider")
                    visible: host || errorCode === ATProtoErrorMsg.PDS_NOT_FOUND
                }

                HostingComboBox {
                    id: hostField
                    Layout.fillWidth: true
                    Layout.leftMargin: 10
                    Layout.rightMargin: 10
                    host: loginPage.host
                    visible: host || errorCode === ATProtoErrorMsg.PDS_NOT_FOUND
                }

                AccessibleLabel {
                    Layout.leftMargin: 10
                    Layout.rightMargin: 10
                    Layout.fillWidth: true
                    color: guiSettings.errorColor
                    wrapMode: Text.Wrap
                    text: "⚠️ Failed to resolve your identity. Can you set your hosting provider?"
                    visible: errorCode === ATProtoErrorMsg.PDS_NOT_FOUND
                }

                AccessibleText {
                    Layout.fillWidth: true
                    topPadding: 10
                    leftPadding: 10
                    font.bold: true
                    color: guiSettings.textColor
                    text: qsTr("Account")
                }

                SkyTextInput {
                    id: userField
                    Layout.fillWidth: true
                    Layout.leftMargin: 10
                    Layout.rightMargin: 10
                    parentFlick: flick
                    enabled: isNewAccount()
                    svgIcon: SvgOutline.atSign
                    initialText: user
                    placeholderText: qsTr("User name")
                    inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText | Qt.ImhSensitiveData
                    maximumLength: 253
                    validator: RegularExpressionValidator { regularExpression: /([a-zA-Z0-9]([a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?\.)+[a-zA-Z]([a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?/ }
                }

                SkyTextInput {
                    id: passwordField
                    Layout.fillWidth: true
                    Layout.leftMargin: 10
                    Layout.rightMargin: 10
                    parentFlick: flick
                    svgIcon: SvgFilled.lock
                    initialText: password
                    echoMode: TextInput.Password
                    placeholderText: qsTr("Password")
                    inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText | Qt.ImhSensitiveData
                    maximumLength: 255

                    SkyButton {
                        anchors.right: parent.right
                        implicitHeight: 40
                        textColor: guiSettings.linkColor
                        Material.background: "transparent"
                        text: qsTr("Forgot?")
                        visible: passwordField.text.length === 0 && !isNewAccount()
                        onClicked: forgotPassword()
                    }
                }

                AccessibleCheckBox {
                    id: rememberPasswordSwitch
                    leftPadding: 10
                    text: qsTr("Remember password")
                    checked: !isNewAccount() && userSettings.getRememberPassword(did)
                    onCheckedChanged: {
                        if (!isNewAccount())
                            userSettings.setRememberPassword(did, checked)
                    }
                }

                AccessibleText {
                    Layout.fillWidth: true
                    topPadding: 10
                    leftPadding: 10
                    font.bold: true
                    color: guiSettings.textColor
                    text: qsTr("2FA Confirmation")
                    visible: authFactorTokenRequired()
                }

                SkyTextInput {
                    id: authFactorTokenField
                    Layout.fillWidth: true
                    Layout.leftMargin: 10
                    Layout.rightMargin: 10
                    parentFlick: flick
                    svgIcon: SvgOutline.confirmationCode
                    placeholderText: qsTr("Confirmation code")
                    inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText | Qt.ImhSensitiveData
                    maximumLength: 253
                    visible: authFactorTokenRequired()
                }

                AccessibleText {
                    Layout.leftMargin: 10
                    Layout.rightMargin: 10
                    Layout.fillWidth: true
                    color: guiSettings.textColor
                    wrapMode: Text.Wrap
                    font.pointSize: guiSettings.scaledFont(7/8)
                    text: qsTr("Check your email for a login code and enter it here.")
                    visible: authFactorTokenRequired()
                }

                AccessibleLabel {
                    id: msgLabel
                    Layout.leftMargin: 10
                    Layout.rightMargin: 10
                    Layout.fillWidth: true
                    color: guiSettings.errorColor
                    wrapMode: Text.Wrap
                    text: "⚠️ " + errorMsg
                    visible: errorMsg && mustShowError()
                }
            }

            AccessibleText {
                anchors.left: parent.left
                anchors.leftMargin: 10
                anchors.top: loginForm.bottom
                anchors.topMargin: 10
                textFormat: Text.RichText
                text: qsTr(`<a href="settings" style="color: ${guiSettings.linkColor}; text-decoration: none">Advanced settings</a>`)
                onLinkActivated: editAdvancedSettings()
            }

            SkyButton {
                id: okButton
                anchors.top: loginForm.bottom
                anchors.right: loginForm.right
                anchors.rightMargin: 10
                height: 40
                text: qsTr("OK")
                enabled: (!hostField.visible || hostField.editText) &&
                         userField.text &&
                         passwordField.text &&
                         (!authFactorTokenRequired() || authFactorTokenField.text)
                onClicked: {
                    const handle = autoCompleteHandle(userField.text, hostField.editText)
                    loginPage.accepted(false,
                                       hostField.visible ? hostField.editText : "",
                                       handle, passwordField.text,
                                       loginPage.did,
                                       rememberPasswordSwitch.checked,
                                       authFactorTokenField.text,
                                       loginPage.setAdvancedSettings,
                                       loginPage.serviceAppView,
                                       loginPage.serviceChat,
                                       loginPage.serviceVideoHost,
                                       loginPage.serviceVideoDid)
                }
            }
        }

        Flickable {
            id: oauthFlick
            contentHeight: oauthOkButton.y + oauthOkButton.height
            flickableDirection: Flickable.VerticalFlick
            boundsBehavior: Flickable.StopAtBounds

            ColumnLayout {
                id: oauthLoginForm
                width: parent.width
                Accessible.role: Accessible.Pane

                AccessibleText {
                    Layout.fillWidth: true
                    topPadding: 10
                    leftPadding: 10
                    rightPadding: 10
                    text: qsTr("Login via the web site of your provider")
                }

                AccessibleText {
                    Layout.fillWidth: true
                    topPadding: 10
                    leftPadding: 10
                    font.bold: true
                    text: qsTr("Hosting provider")
                    visible: host || errorCode === ATProtoErrorMsg.PDS_NOT_FOUND
                }

                HostingComboBox {
                    id: oauthHostField
                    Layout.fillWidth: true
                    Layout.leftMargin: 10
                    Layout.rightMargin: 10
                    host: loginPage.host
                    visible: host || errorCode === ATProtoErrorMsg.PDS_NOT_FOUND
                }

                AccessibleLabel {
                    Layout.leftMargin: 10
                    Layout.rightMargin: 10
                    Layout.fillWidth: true
                    color: guiSettings.errorColor
                    wrapMode: Text.Wrap
                    text: "⚠️ Failed to resolve your identity. Can you set your hosting provider?"
                    visible: errorCode === ATProtoErrorMsg.PDS_NOT_FOUND
                }

                AccessibleText {
                    Layout.fillWidth: true
                    topPadding: 10
                    leftPadding: 10
                    font.bold: true
                    color: guiSettings.textColor
                    text: qsTr("Account")
                }

                SkyTextInput {
                    id: oauthUserField
                    Layout.fillWidth: true
                    Layout.leftMargin: 10
                    Layout.rightMargin: 10
                    parentFlick: oauthFlick
                    enabled: isNewAccount()
                    svgIcon: SvgOutline.atSign
                    initialText: user
                    placeholderText: qsTr("User name")
                    inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText | Qt.ImhSensitiveData
                    maximumLength: 253
                    validator: RegularExpressionValidator { regularExpression: /([a-zA-Z0-9]([a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?\.)+[a-zA-Z]([a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?/ }
                }

                AccessibleLabel {
                    id: oauthMsgLabel
                    Layout.leftMargin: 10
                    Layout.rightMargin: 10
                    Layout.fillWidth: true
                    color: guiSettings.errorColor
                    wrapMode: Text.Wrap
                    text: "⚠️ " + errorMsg
                    visible: errorMsg && mustShowError()

                    Accessible.role: Accessible.AlertMessage
                    Accessible.name: text
                    Accessible.description: Accessible.name
                }
            }

            AccessibleText {
                anchors.left: parent.left
                anchors.leftMargin: 10
                anchors.top: oauthLoginForm.bottom
                anchors.topMargin: 10
                textFormat: Text.RichText
                text: qsTr(`<a href="settings" style="color: ${guiSettings.linkColor}; text-decoration: none">Advanced settings</a>`)
                onLinkActivated: editAdvancedSettings()
            }

            SkyButton {
                id: oauthOkButton
                anchors.top: oauthLoginForm.bottom
                anchors.right: oauthLoginForm.right
                anchors.rightMargin: 10
                height: 40
                text: qsTr("OK")
                enabled: (!oauthHostField.visible || oauthHostField.editText) &&
                         oauthUserField.text &&
                         !redirectTimer.running
                onClicked: {
                    skywalker.showStatusMessage(qsTr("Redirecting to login page"), QEnums.STATUS_LEVEL_INFO, 5)
                    redirectTimer.start()
                    const handle = autoCompleteHandle(oauthUserField.text, oauthHostField.editText)
                    loginPage.accepted(true,
                                       oauthHostField.visible ? oauthHostField.editText : "",
                                       handle, "",
                                       loginPage.did,
                                       false,
                                       "",
                                       loginPage.setAdvancedSettings,
                                       loginPage.serviceAppView,
                                       loginPage.serviceChat,
                                       loginPage.serviceVideoHost,
                                       loginPage.serviceVideoDid)
                }
            }

            Timer {
                id: redirectTimer
                interval: 5000
            }
        }
    }

    VirtualKeyboardHandler {
        id: keyboardHandler
    }

    AccountUtils {
        id: accountUtils
        skywalker: loginPage.skywalker

        onRequestResetPasswordOk: enterPasswordResetToken()
        onRequestResetPasswordFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)

        onResetPasswordOk: (password) => {
            if (userSettings.getRememberPassword(did))
                userSettings.savePassword(did, password)

            skywalker.showStatusMessage(qsTr("Password changed"), QEnums.STATUS_LEVEL_INFO)
        }

        onResetPasswordFailed: (error) => skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
    }

    function forgotPassword() {
        let component = guiSettings.createComponent("ForgotPasswordDialog.qml")
        let dialog = component.createObject(loginPage, { host: hostField.editText })
        dialog.onHostEmail.connect((host, email) => { dialog.close(); accountUtils.requestPasswordReset(email, host) })
        dialog.onAlreadyHasCode.connect(() => { dialog.close(); enterPasswordResetToken() })
        dialog.onRejected.connect(() => { dialog.close() })
        dialog.open()
    }

    function enterPasswordResetToken() {
        guiSettings.askPasswordResetToken(root,
            (password, token) => { accountUtils.resetPassword(password, token, hostField.editText) }
        )
    }

    function autoCompleteHandle(handle, host) {
        let newHandle = handle

        if (newHandle.charAt(0) === "@")
            newHandle = newHandle.slice(1)

        if (!newHandle.includes("."))
            newHandle = newHandle + "." + host

        return newHandle
    }

    function isNewAccount() {
        return did.length === 0
    }

    function authFactorTokenRequired() {
        return errorCode === ATProtoErrorMsg.AUTH_FACTOR_TOKEN_REQUIRED || errorCode === ATProtoErrorMsg.INVALID_TOKEN
    }

    function mustShowError() {
        return errorCode !== ATProtoErrorMsg.AUTH_FACTOR_TOKEN_REQUIRED &&
                errorCode !== ATProtoErrorMsg.PDS_NOT_FOUND
    }

    function editAdvancedSettings() {
        let component = guiSettings.createComponent("AdvancedSettingsForm.qml")
        let form = component.createObject(loginPage, { newUser: isNewAccount(), userDid: did })
        form.onSettings.connect((serviceAppView, serviceChat, serviceVideoHost, serviceVideoDid) => {
                loginPage.serviceAppView = serviceAppView
                loginPage.serviceChat = serviceChat
                loginPage.serviceVideoHost = serviceVideoHost
                loginPage.serviceVideoDid = serviceVideoDid
                loginPage.setAdvancedSettings = true
        })
        form.onClosed.connect(() => { popStack() })
        pushStack(form)
    }

    function closed() {
        canceled()
    }

    Component.onCompleted: {
        hostField.init()
        oauthHostField.init()

        if (useOAuth) {
            authBar.setCurrentIndex(1)
        } else {
            if (authFactorTokenRequired())
                authFactorTokenField.setFocus()
            else if (isNewAccount())
                userField.setFocus()
            else
                passwordField.setFocus()
        }


        if (errorMsg && mustShowError())
            skywalker.showStatusMessage(errorMsg, QEnums.STATUS_LEVEL_ERROR)
    }
}
