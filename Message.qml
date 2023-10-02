import QtQuick
import QtQuick.Controls

Dialog {
    id: msgDialog
    modal: true
    standardButtons: Dialog.Ok
    anchors.centerIn: parent

    Label {
        id: msgLabel
        anchors.fill: parent
        textFormat: Text.StyledText
        wrapMode: Text.Wrap
    }

    function show(msg) {
        msgLabel.text = msg;
        open();
    }
}
