import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.settings

Dialog {
    property string user
    property string password
    property string host

    title: qsTr("Login")
    width: parent.width
    standardButtons: Dialog.Ok
    modal: true

    GridLayout {
        columns: 2
        anchors.fill: parent

        Label {
            text: qsTr("User:")
        }
        TextField {
            id: userField
            Layout.fillWidth: true
            text: "michelbestaat.bsky.social"
            focus: true
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
            text: qsTr("Host:")
        }
        TextField {
            id: hostField
            Layout.fillWidth: true
            text: "bsky.social"
        }

        Label {
            id: msgLabel
            Layout.columnSpan: 2
            Layout.fillWidth: true
            color: "red"
            wrapMode: Text.Wrap
        }

        Settings {
            property alias user: userField.text
            property alias password: passwordField.text
            property alias host: hostField.text
        }
    }

    onAccepted: {
        user = userField.text
        password = passwordField.text
        host = hostField.text
    }

    function show(msg = "") {
        if (msg) {
            msgLabel.text = msg
            msgLabel.visible = true
        } else {
            msgLabel.visible = false
        }

        open()
    }
}
