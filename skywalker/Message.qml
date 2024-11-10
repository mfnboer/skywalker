import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    property string emoji: ""

    id: msgDialog
    contentHeight: msgRow.height
    width: parent.width - 40
    modal: true
    standardButtons: Dialog.Ok
    anchors.centerIn: parent
    Material.background: guiSettings.backgroundColor

    onOpened: msgLabel.focus = true

    RowLayout {
        id: msgRow
        width: parent.width
        spacing: 10

        Label {
            id: emojiLabel
            height: visible ? implicitHeight : 0
            verticalAlignment: Text.AlignVCenter
            font.pointSize: guiSettings.scaledFont(6)
            text: emoji
            visible: emoji
        }

        Label {
            id: msgLabel
            Layout.fillWidth: true

            verticalAlignment: Text.AlignVCenter
            padding: 10
            textFormat: Text.StyledText
            wrapMode: Text.Wrap

            Accessible.role: Accessible.StaticText
            Accessible.name: text
            Accessible.description: Accessible.name
        }
    }


    function show(msg) {
        msgLabel.text = msg
        open()
    }
}
