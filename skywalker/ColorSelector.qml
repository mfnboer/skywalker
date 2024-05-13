import QtQuick.Dialogs

ColorDialog {
    onAccepted: destroy();
    onRejected: destroy();
}
