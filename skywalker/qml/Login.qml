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
    property bool setAdvancedSettings: false
    property string serviceAppView
    property string serviceChat
    property string serviceVideoHost
    property string serviceVideoDid
    property var userSettings: root.getSkywalker().getUserSettings()
    readonly property string sideBarTitle: isNewAccount() ? qsTr("Add Account") : qsTr("Login")

    signal accepted(string host,
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

    Flickable {
        id: flick
        anchors.fill: parent
        clip: true
        contentWidth: parent.width
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
                color: guiSettings.textColor
                text: qsTr("Sign into")
            }

            ComboBox {
                id: hostField
                Layout.fillWidth: true
                Layout.leftMargin: 10
                Layout.rightMargin: 10
                model: ["bsky.social"]
                editable: true
                editText: host
                inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
                enabled: isNewAccount()
                activeFocusOnTab: false

                Accessible.role: Accessible.ComboBox
                Accessible.name: qsTr(`Sign into network ${editText}`)
                Accessible.description: qsTr("Choose network to sign into")
                Accessible.editable: enabled

                Rectangle {
                    z: parent.z - 1
                    width: hostField.width
                    height: hostField.height
                    radius: 5
                    color: guiSettings.textInputBackgroundColor
                }
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
            }

            AccessibleCheckBox {
                id: rememberPasswordSwitch
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
                visible: errorMsg && errorCode !== ATProtoErrorMsg.AUTH_FACTOR_TOKEN_REQUIRED

                Accessible.role: Accessible.AlertMessage
                Accessible.name: text
                Accessible.description: Accessible.name
            }
        }

        SkyButton {
            anchors.top: loginForm.bottom
            text: qsTr("Advanced settings")
            onClicked: editAdvancedSettings()
        }

        SkyButton {
            id: okButton
            anchors.top: loginForm.bottom
            anchors.right: parent.right
            text: qsTr("OK")
            enabled: hostField.editText && userField.text && passwordField.text && (!authFactorTokenRequired() || authFactorTokenField.text)
            onClicked: {
                const handle = autoCompleteHandle(userField.text, hostField.editText)
                loginPage.accepted(hostField.editText,
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

    VirtualKeyboardHandler {
        id: keyboardHandler
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
        if (authFactorTokenRequired())
            authFactorTokenField.setFocus()
        else if (isNewAccount())
            userField.setFocus()
        else
            passwordField.setFocus()

        if (errorMsg && errorCode !== ATProtoErrorMsg.AUTH_FACTOR_TOKEN_REQUIRED)
            skywalker.showStatusMessage(errorMsg, QEnums.STATUS_LEVEL_ERROR)
    }
}
