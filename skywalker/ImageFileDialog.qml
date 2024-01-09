import QtCore
import QtQuick
import QtQuick.Dialogs

FileDialog {
    signal fileSelected(string fileName)

    id: fileDialog
    currentFolder: StandardPaths.standardLocations(StandardPaths.PicturesLocation)[0]
    nameFilters: ["Image files (*.jpg *.jpeg *.png *.webp *.gif)"]
    onAccepted: {
        let fileName = selectedFile.toString()
        if (!fileName.startsWith("file://"))
            fileName = "file://" + fileName

        fileSelected(fileName)
    }
}
