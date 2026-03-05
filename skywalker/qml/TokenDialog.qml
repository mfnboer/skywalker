import QtQuick
import QtQuick.Controls
import skywalker

Dialog {
    signal token(string token)

    id: dialog
    topMargin: guiSettings.headerHeight
    leftMargin: 20
    width: parent.width - 2 * leftMargin
    contentHeight: col.height
    modal: true
    Material.background: guiSettings.backgroundColor

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
            svgIcon: SvgOutline.confirmationCode
            placeholderText: "XXXXX-XXXXX"
            inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText | Qt.ImhSensitiveData
            maximumLength: 253
            valid: text.length > 0
        }
    }

    footer: DialogButtonBox {
        SkyTransparentButton {
            text: qsTr("Cancel")
            onClicked: dialog.reject()
        }
        SkyTransparentButton {
            text: qsTr("OK")
            enabled: tokenField.valid
            onClicked: dialog.handleOk()
        }
    }

    function handleOk() {
        accept()
        token(tokenField.text)
    }
}
