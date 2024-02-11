import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Dialog {
    required property string contentAuthorDid
    required property contentlabel label
    readonly property string contentWarning: skywalker.getContentWarning([label])

    id: contentLabelInfo
    width: parent.width
    contentHeight: grid.height
    title: qsTr("Content label")
    modal: true
    standardButtons: Dialog.Ok
    anchors.centerIn: parent

    GridLayout {
        id: grid
        width: parent.width
        columns: 2
        columnSpacing: 10

        Text {
            font.bold: true
            color: guiSettings.textColor
            text: qsTr("Label:")
        }
        Text {
            Layout.fillWidth: true
            color: guiSettings.textColor
            elide: Text.ElideRight
            text: label.text
        }

        Text {
            font.bold: true
            color: guiSettings.textColor
            text: qsTr("Creator:")
        }
        Text {
            id: creatorText
            Layout.fillWidth: true
            color: guiSettings.textColor
            elide: Text.ElideRight
            text: contentAuthorDid === label.did ? qsTr("User") : qsTr("Moderator")
        }

        Text {
            font.bold: true
            color: guiSettings.textColor
            text: qsTr("Date:")
        }
        Text {
            Layout.fillWidth: true
            color: guiSettings.textColor
            elide: Text.ElideRight
            text: label.createdAt.toLocaleString(Qt.locale(), Locale.LongFormat)
        }
    }

    GuiSettings {
        id: guiSettings
    }
}
