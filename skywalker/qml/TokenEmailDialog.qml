import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import skywalker

Dialog {
    required property bool tokenRequired
    property Skywalker skywalker: root.getSkywalker()

    signal emailToken(string email, string token)

    id: dialog
    topMargin: guiSettings.headerHeight
    leftMargin: 20
    width: parent.width - 2 * leftMargin
    contentHeight: col.height
    title: qsTr("Update email")
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
                wrapMode: Text.Wrap
                text: qsTr("Enter token you received by email")
                visible: tokenRequired
            }

            SkyTextInput {
                id: tokenField
                width: parent.width
                parentFlick: flick
                svgIcon: SvgOutline.confirmationCode
                placeholderText: "XXXXX-XXXXX"
                inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText | Qt.ImhSensitiveData
                maximumLength: 253
                valid: text.length > 0 || !tokenRequired
                visible: tokenRequired
            }

            AccessibleText {
                width: parent.width
                wrapMode: Text.Wrap
                text: qsTr("New email address")
            }

            SkyTextInput {
                id: emailField
                width: parent.width
                svgIcon: SvgFilled.atSign
                placeholderText: qsTr(`Enter your new email address`)
                inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText | Qt.ImhSensitiveData
                validator: RegularExpressionValidator { regularExpression: /[^ ]+@[^ ]+/ }
                valid: text.indexOf("@") > 0 && text.indexOf("@") < text.length - 1
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
            enabled: tokenField.valid && emailField.valid
            onClicked: dialog.handleOk()
        }
    }

    function handleOk() {
        accept()
        emailToken(emailField.text, tokenField.text)
    }
}
