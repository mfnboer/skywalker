import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Page {
    property string did
    property string host
    property string user
    property string error

    signal accepted(string host, string handle, string password, string did)
    signal canceled

    id: loginPage
    width: parent.width

    Accessible.role: Accessible.Pane

    header: SimpleHeader {
        text: isNewAccount() ? qsTr("Add Account") : qsTr("Login")
        onBack: loginPage.canceled()
    }

    ColumnLayout {
        id: loginForm
        width: parent.width

        Text {
            Layout.fillWidth: true
            topPadding: 10
            leftPadding: 10
            font.bold: true
            color: guiSettings.textColor
            text: qsTr("Sign into")

            Accessible.role: Accessible.StaticText
            Accessible.name: text
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
            Accessible.editable: enabled
        }

        Text {
            Layout.fillWidth: true
            topPadding: 10
            leftPadding: 10
            font.bold: true
            color: guiSettings.textColor
            text: qsTr("Account")

            Accessible.role: Accessible.StaticText
            Accessible.name: text
        }

        SkyTextInput {
            id: userField
            Layout.fillWidth: true
            Layout.leftMargin: 10
            Layout.rightMargin: 10
            enabled: isNewAccount()
            focus: true
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
            echoMode: TextInput.Password
            placeholderText: qsTr("Password")
            inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText | Qt.ImhSensitiveData
            maximumLength: 255
        }

        Label {
            id: msgLabel
            Layout.leftMargin: 10
            Layout.rightMargin: 10
            Layout.fillWidth: true
            color: guiSettings.errorColor
            wrapMode: Text.Wrap
            text: error
            visible: error

            Accessible.role: Accessible.AlertMessage
            Accessible.name: text
        }
    }

    SkyButton {
        anchors.top: loginForm.bottom
        anchors.right: parent.right
        text: qsTr("OK")
        enabled: hostField.editText && userField.text && passwordField.text
        onClicked: {
            const handle = autoCompleteHandle(userField.text, hostField.editText)
            loginPage.accepted(hostField.editText, handle, passwordField.text, loginPage.did)
        }

        Accessible.role: Accessible.Button
        Accessible.name: text
        Accessible.onPressAction: if (enabled) clicked()
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

    Component.onCompleted: {
        userField.setFocus()
    }
}
