import QtQuick.Dialogs

ColorDialog {
    title: qsTr("Select color")
    modality: Qt.WindowModal
    onAccepted: destroy();
    onRejected: destroy();
}
