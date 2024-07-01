import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker
import atproto

Page {
    property string did
    property string host
    property string user
    property string errorCode
    property string errorMsg
    property string password
    property var userSettings: root.getSkywalker().getUserSettings()

    signal accepted(string host, string handle, string password, string did, bool rememberPassword, string authFactorTokenField)
    signal canceled

    id: loginPage
    width: parent.width

    Accessible.role: Accessible.Pane

    header: SimpleHeader {
        text: isNewAccount() ? qsTr("Add Account") : qsTr("Login")
        onBack: loginPage.canceled()
    }

    Flickable {
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
                enabled: isNewAccount()
                svgIcon: svgOutline.atSign
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
                svgIcon: svgFilled.lock
                initialText: password
                echoMode: TextInput.Password
                placeholderText: qsTr("Password")
                inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText | Qt.ImhSensitiveData
                maximumLength: 255
            }

            AccessibleSwitch {
                id: rememberPasswordSwitch
                text: qsTr("Remember password")
                checked: !isNewAccount() && userSettings.getRememberPassword(did)
                onCheckedChanged: {
                    if (!isNewAccount())
                        userSettings.setRememberPassword(did)
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
                svgIcon: svgOutline.confirmationCode
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

            Label {
                id: msgLabel
                Layout.leftMargin: 10
                Layout.rightMargin: 10
                Layout.fillWidth: true
                color: guiSettings.errorColor
                wrapMode: Text.Wrap
                text: errorMsg
                visible: errorMsg && errorCode !== ATProtoErrorMsg.AUTH_FACTOR_TOKEN_REQUIRED

                Accessible.role: Accessible.AlertMessage
                Accessible.name: text
                Accessible.description: Accessible.name
            }
        }

        SkyButton {
            id: okButton
            anchors.top: loginForm.bottom
            anchors.right: parent.right
            text: qsTr("OK")
            enabled: hostField.editText && userField.text && passwordField.text && (!authFactorTokenRequired() || authFactorTokenField.text)
            onClicked: {
                const handle = autoCompleteHandle(userField.text, hostField.editText)
                loginPage.accepted(hostField.editText, handle, passwordField.text, loginPage.did, rememberPasswordSwitch.checked, authFactorTokenField.text)
            }
        }
    }

    VirtualKeyboardPageResizer {
        id: virtualKeyboardPageResizer
    }

    GuiSettings {
        id: guiSettings
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

    Component.onCompleted: {
        virtualKeyboardPageResizer.fullPageHeight = parent.height

        if (authFactorTokenRequired())
            authFactorTokenField.setFocus()
        else if (isNewAccount())
            userField.setFocus()
        else
            passwordField.setFocus()

        if (errorMsg && errorCode !== ATProtoErrorMsg.AUTH_FACTOR_TOKEN_REQUIRED)
            statusPopup.show(errorMsg, QEnums.STATUS_LEVEL_ERROR)
    }
}
