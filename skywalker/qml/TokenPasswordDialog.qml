import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import skywalker

Dialog {
    property Skywalker skywalker: root.getSkywalker()
    readonly property int minPasswordLength: 8

    signal passwordToken(string password, string token)

    id: dialog
    width: parent.width - 40
    contentHeight: col.height
    title: qsTr("Reset password")
    modal: true
    //standardButtons: Dialog.Ok | Dialog.Cancel
    anchors.centerIn: parent
    Material.background: guiSettings.backgroundColor

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
            }
        }
    }

    footer: DialogButtonBox {
        Button {
            Material.background: "transparent"
            text: qsTr("Cancel")
            display: AbstractButton.TextOnly
            icon.name: ""
            icon.source: ""
            onClicked: dialog.reject()
        }
        Button {
            Material.background: "transparent"
            text: qsTr("OK")
            display: AbstractButton.TextOnly
            icon.name: ""
            icon.source: ""
            onClicked: dialog.handleOk()
        }
    }

    function handleOk() {
        if (!tokenField.displayText) {
            skywalker.showStatusMessage(qsTr("Token missing"), QEnums.STATUS_LEVEL_ERROR)
            return
        }

        if (passwordField.displayText.length < minPasswordLength) {
            skywalker.showStatusMessage(qsTr(`Password must be at least ${minPasswordLength} characters`), QEnums.STATUS_LEVEL_ERROR)
            return
        }

        accept()
        passwordToken(passwordField.displayText, tokenField.displayText)
    }
}
