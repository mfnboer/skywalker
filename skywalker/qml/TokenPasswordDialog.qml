import QtQuick
import QtQuick.Controls
import skywalker

SkyDialog {
    property Skywalker skywalker: root.getSkywalker()
    readonly property int minPasswordLength: 8

    signal passwordToken(string password, string token)

    id: dialog
    topMargin: guiSettings.headerHeight
    leftMargin: 20
    width: parent.width - 2 * leftMargin
    contentHeight: col.height
    title: qsTr("Reset password")

    Flickable {
        id: flick
        anchors.fill: parent
        clip: true
        contentWidth: parent.width
        contentHeight: col.height
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: SkyScrollBarVertical {}

        Column {
            id: col
            x: 10
            width: parent.width - 20
            spacing: 10

            AccessibleText {
                width: parent.width
                wrapMode: Text.Wrap
                text: qsTr("Enter token you received by email")
            }

            SkyTextInput {
                id: tokenField
                width: parent.width
                parentFlick: flick
                svgIcon: SvgOutline.confirmationCode
                placeholderText: "XXXXX-XXXXX"
                inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText | Qt.ImhSensitiveData
                maximumLength: 253
                valid: text.length > 0
            }

            AccessibleText {
                width: parent.width
                wrapMode: Text.Wrap
                text: qsTr("New password")
            }

            SkyTextInput {
                id: passwordField
                width: parent.width
                svgIcon: SvgFilled.lock
                echoMode: TextInput.Password
                placeholderText: qsTr(`At least ${minPasswordLength} characters`)
                inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText | Qt.ImhSensitiveData
                maximumLength: 255
                valid: text.length >= minPasswordLength
            }

            SkyTextInput {
                id: verificationField
                width: parent.width
                svgIcon: SvgFilled.lock
                echoMode: TextInput.Password
                placeholderText: qsTr("Repeat new password")
                inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText | Qt.ImhSensitiveData
                maximumLength: 255
                valid: text.length >= minPasswordLength
            }
        }
    }

    footer: DialogButtonBox {
        SkyTransparentButton {
            text: qsTr("Cancel")
            onClicked: dialog.reject()
        }
        SkyTransparentButton {
            text: qsTr("OK")
            enabled: tokenField.valid && passwordField.valid && verificationField.valid
            onClicked: dialog.handleOk()
        }
    }

    function handleOk() {
        if (passwordField.text !== verificationField.text)
        {
            skywalker.showStatusMessage(qsTr("Passwords are not equal"), QEnums.STATUS_LEVEL_ERROR)
            return
        }

        accept()
        passwordToken(passwordField.text, tokenField.text)
    }
}
