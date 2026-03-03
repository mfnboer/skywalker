import QtQuick
import QtQuick.Controls

Dialog {
    id: dialog
    width: parent.width - 40
    contentHeight: col.height
    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel
    anchors.centerIn: parent
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
        }
    }

    function getToken() {
        return tokenField.displayText
    }
}
