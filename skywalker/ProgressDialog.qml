import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: progressDialog
    contentHeight: column.height
    width: parent.width - 40
    modal: true
    standardButtons: Dialog.Cancel
    anchors.centerIn: parent
    Material.background: guiSettings.backgroundColor

    ColumnLayout {
        id: column
        width: parent.width
        spacing: 10

        AccessibleText {
            id: progressText
            Layout.fillWidth: true
            wrapMode: Text.Wrap
        }

        ProgressBar {
            id: progressBar
            Layout.fillWidth: true
        }
    }

    GuiSettings {
        id: guiSettings
    }

    function show(msg) {
        progressText.text = msg
        open()
    }

    function setProgress(progress) {
        progressBar.value = progress
    }
}
