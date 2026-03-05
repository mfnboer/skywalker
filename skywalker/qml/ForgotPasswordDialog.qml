import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import skywalker

Dialog {
    required property string host
    property Skywalker skywalker: root.getSkywalker()

    signal hostEmail(string host, string email)
    signal alreadyHasCode

    id: dialog
    topMargin: guiSettings.headerHeight
    leftMargin: 20
    width: parent.width - 2 * leftMargin
    contentHeight: col.height
    title: qsTr("Forgot password")
    modal: true
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
                text: qsTr("Hosting provider")
            }

            HostingComboBox {
                id: hostField
                width: parent.width
                host: dialog.host
                valid: editText.length > 0
            }

            AccessibleText {
                width: parent.width
                text: qsTr("Email address")
            }

            SkyTextInput {
                id: emailField
                width: parent.width
                parentFlick: flick
                svgIcon: SvgOutline.atSign
                placeholderText: qsTr("Enter your email address")
                inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText | Qt.ImhSensitiveData
                maximumLength: 253
                validator: RegularExpressionValidator { regularExpression: /[^ ]+@[^ ]+/ }
                valid: text.indexOf("@") > 0 && text.indexOf("@") < text.length - 1
            }

            AccessibleText {
                width: parent.width
                wrapMode: Text.Wrap
                text: qsTr("Enter the email address you used to create your account. You will receive a \"reset code\" to set a new password")
            }

            AccessibleText {
                width: parent.width
                textFormat: Text.RichText
                text: qsTr(`<a href="settings" style="color: ${guiSettings.linkColor}; text-decoration: none">I already have a code</a>.`)
                onLinkActivated: handleHasCode()
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
            enabled: hostField.valid && emailField.valid
            onClicked: dialog.handleOk()
        }
    }

    function handleOk() {
        accept()
        hostEmail(hostField.editText, emailField.text)
    }

    function handleHasCode() {
        accept()
        alreadyHasCode()
    }

    Component.onCompleted: {
        hostField.init()
    }
}
