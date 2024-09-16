import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Dialog {
    required property videouploadlimits limits

    id: limitsDialog
    width: parent.width - 40
    title: qsTr("Video upload limits")
    modal: true
    contentHeight: limitsGrid.height
    standardButtons: Dialog.Ok
    anchors.centerIn: parent
    Material.background: guiSettings.backgroundColor

    GridLayout {
        id: limitsGrid
        width: parent.width
        columns: 2

        AccessibleText {
            text: qsTr("Can upload:")
        }

        AccessibleText {
            text: limits.canUpload ? qsTr("Yes") : qsTr("No")
        }

        AccessibleText {
            text: qsTr("Remaining videos today:")
        }

        AccessibleText {
            text: limits.remainingDailyVideos
        }

        AccessibleText {
            text: qsTr("Remaining bytes today:")
        }

        AccessibleText {
            text: getBytesString(limits.remainingDailyBytes)
        }

        AccessibleText {
            Layout.columnSpan: 2
            text: getMessageOrError()
            visible: !limits.canUpload
        }
    }

    GuiSettings {
        id: guiSettings
    }

    function getBytesString(bytes) {
        const mb = bytes / 1000000

        if (mb < 1000)
            return `${(mb.toFixed(1))} MB`

        const gb = mb / 1000
        return `${(gb.toFixed(1))} GB`
    }

    function getMessageOrError() {
        if (limits.message)
            return limits.message

        return message.error
    }
}
