import QtQuick
import QtQuick.Dialogs
import skywalker

FileDialog {
    property Skywalker skywalker: root.getSkywalker()

    id: backupFileDialog
    nameFilters: ["JSON files (*.json)"]
    defaultSuffix: "json"

    onAccepted: {
        console.debug("File selected for backup/restore:", selectedFile)

        if (fileMode === FileDialog.OpenFile)
            loadSettings()
        else if (fileMode === FileDialog.SaveFile)
            saveSettings()
        else
            console.warn("Unexpected file mode:", fileMode)
    }

    function saveSettings() {
        let fileName = ""

        if (Qt.platform.os === "android") {
            fileName = FileUtils.resolveContentUriToFileName(selectedFile)

            if (fileName.startsWith("(invalid)")) {
                skywalker.showStatusMessage(qsTr("Enter a file name or select an existing file to save settings."), QEnums.STATUS_LEVEL_ERROR)
                FileUtils.deleteContentUri(selectedFile)
                return
            }
        } else {
            fileName = selectedFile.toString().slice(7)
        }

        const error = skywalker.getUserSettings().save(selectedFile)

        if (error)
            skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
        else
            skywalker.showStatusMessage(qsTr(`Settings saved to: ${fileName}`), QEnums.STATUS_LEVEL_INFO)
    }

    function loadSettings() {
        root.signOutCurrentUser()
        const error = skywalker.getUserSettings().load(selectedFile)

        if (error)
            skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
        else
            skywalker.showStatusMessage(qsTr("Restored settings"), QEnums.STATUS_LEVEL_INFO)

        root.signIn()
    }

    function backup() {
        fileMode = FileDialog.SaveFile
        selectedFile = ""
        open()
    }

    function restore() {
        fileMode = FileDialog.OpenFile
        open()
    }

    function help() {
        guiSettings.notice(root, qsTr(
            "Your configured accounts and some of their settings are stored locally on your " +
            "device. When you uninstall Skywalker you may lose these. You can backup these " +
            "settings to a file and restore them again, e.g. after a re-install or on another " +
            "device. Note that <b>all</b> your accounts (if you have multple) will be backed up, " +
            "not just the currently active one.<br><br>" +
            "To backup you have to enter a file name, e.g. 'skywalker', and the settings will be " +
            "saved to a file called 'skywalker.json'. Or you can select a previous backup file " +
            "and that file will be overwritten with the current settings.<br><br>" +
            "To restore you select a backup file. You will be signed out from Skwalker and the " +
            "backup will be restored."
        ))
    }
}
