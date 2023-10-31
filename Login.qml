import QtCore
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    property string host
    property string user
    property string error

    signal accepted(string host, string user, string password)

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

            Text {
                Layout.alignment: Qt.AlignVCenter
                leftPadding: 10
                font.bold: true
                font.pointSize: guiSettings.scaledFont(10/8)
                color: "white"
                text: isNewAccount() ? qsTr("Add Account") : qsTr("Login")
            }
        }
    }

    GridLayout {
        id: loginForm
        columns: 2
        width: parent.width

        Label {
            text: qsTr("Host:")
        }
        ComboBox {
            id: hostField
            Layout.fillWidth: true
            model: ["bsky.social"]
            editable: true
            editText: host
            enabled: isNewAccount()
        }

        Label {
            text: qsTr("User:")
        }
        TextField {
            id: userField
            Layout.fillWidth: true
            focus: true
            text: user
            enabled: isNewAccount()
        }

        Label {
            text: qsTr("Password:")
        }
        TextField {
            id: passwordField
            Layout.fillWidth: true
            echoMode: TextInput.Password
        }

        Label {
            id: msgLabel
            Layout.columnSpan: 2
            Layout.fillWidth: true
            color: "red"
            wrapMode: Text.Wrap
            text: error
            visible: error
        }
    }

    SkyButton {
        anchors.top: loginForm.bottom
        anchors.right: parent.right
        text: qsTr("OK")
        enabled: hostField.editText && userField.text
        onClicked: {
            const handle = autoCompleteHandle(userField.text, hostField.editText)
            accepted(hostField.editText, handle, passwordField.text)
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
        return user.length === 0
    }
}
