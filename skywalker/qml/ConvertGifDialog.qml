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
            display: AbstractButton.TextOnly
            icon.name: ""
            icon.source: ""
            DialogButtonBox.buttonRole: DialogButtonBox.RejectRole

            // HACK
            // When the virtual keyboard still shows in the compose window, then
            // a press on the butten makes the keyboard disappear, the dialog moves
            // and the press does not become a click.
            onPressed: clicked()
        }
        Button {
            id: videoButton
            flat: true
            text: qsTr("Video")
            display: AbstractButton.TextOnly
            icon.name: ""
            icon.source: ""
            DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole

            onPressed: clicked()
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

}
