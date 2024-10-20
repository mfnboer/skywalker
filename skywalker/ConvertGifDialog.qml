import QtQuick
import QtQuick.Controls

Dialog {
    required property string gifSource

    id: dialog
    contentHeight: column.height
    width: parent.width - 40
    modal: true
    anchors.centerIn: parent
    Material.background: guiSettings.backgroundColor

    footer: DialogButtonBox {
        Button {
            flat: true
            text: qsTr("Image")
            DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
        }
        Button {
            flat: true
            text: qsTr("Video")
            DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
        }
    }

    Column {
        id: column
        width: parent.width
        spacing: 10

        AccessibleText {
            width: parent.width
            wrapMode: Text.Wrap
            text: qsTr("Do you want to post this GIF as still image or video?")
        }

        Image {
            width: parent.width
            fillMode: Image.PreserveAspectFit
            source: gifSource
        }
    }

    GuiSettings {
        id: guiSettings
    }
}
