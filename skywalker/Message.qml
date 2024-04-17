import QtQuick
import QtQuick.Controls

Dialog {
    id: msgDialog
    modal: true
    standardButtons: Dialog.Ok
    anchors.centerIn: parent

    onOpened: msgLabel.focus = true

    Label {
        id: msgLabel
        anchors.fill: parent
        textFormat: Text.StyledText
        wrapMode: Text.Wrap

        Accessible.role: Accessible.StaticText
        Accessible.name: text
        Accessible.description: Accessible.name
    }

    function show(msg) {
        msgLabel.text = msg;
        open();
    }
}
