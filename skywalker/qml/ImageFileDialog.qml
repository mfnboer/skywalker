import QtCore
import QtQuick
import QtQuick.Dialogs

FileDialog {
    signal imageSelected(string fileName)
    signal videoSelected(string fileName)

    id: fileDialog
    currentFolder: StandardPaths.standardLocations(StandardPaths.PicturesLocation)[0]

    onAccepted: {
        let fileName = selectedFile.toString()
        if (!fileName.startsWith("file://"))
            fileName = "file://" + fileName

        if (fileName.endsWith(".mp4"))
            videoSelected(fileName)
        else
            imageSelected(fileName)
    }

    function pick(video) {
        nameFilters = ["Image files (*.jpg *.jpeg *.png *.webp *.gif)"]

        if (video)
            nameFilters.push("Video file (*.mp4)")

        open()
    }
}
