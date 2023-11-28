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

    header: Rectangle {
        width: parent.width
        height: guiSettings.headerHeight
        z: guiSettings.headerZLevel
        color: guiSettings.headerColor

        RowLayout {
            width: parent.width
            height: guiSettings.headerHeight

            SvgButton {
                id: backButton
                iconColor: guiSettings.headerTextColor
                Material.background: "transparent"
                svg: svgOutline.arrowBack
                onClicked: loginPage.canceled()
            }
            Text {
                Layout.alignment: Qt.AlignVCenter
                leftPadding: 10
                font.bold: true
                font.pointSize: guiSettings.scaledFont(10/8)
                color: guiSettings.headerTextColor
                text: isNewAccount() ? qsTr("Add Account") : qsTr("Login")
            }
        }
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
        }

        ComboBox {
            id: hostField
            Layout.fillWidth: true
            Layout.leftMargin: 10
            Layout.rightMargin: 10
            model: ["bsky.social"]
            editable: true
            editText: host
            enabled: isNewAccount()
            activeFocusOnTab: false
        }

        Text {
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
            focus: true
            svgIcon: svgOutline.atSign
            initialText: user
            placeholderText: qsTr("User")
            inputMethodHints: Qt.ImhNoAutoUppercase
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
            inputMethodHints: Qt.ImhNoAutoUppercase
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