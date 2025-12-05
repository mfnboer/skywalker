import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Dialog {
    property string emoji: ""

    signal linkActivated(string link)

    id: msgDialog
    contentHeight: msgRow.height
    width: parent.width - 40
    modal: true
    standardButtons: Dialog.Ok
    anchors.centerIn: parent
    Material.background: guiSettings.backgroundColor

    onOpened: msgLabel.focus = true

    Flickable {
        anchors.fill: parent
        clip: true
        contentWidth: parent.width
        contentHeight: msgRow.height
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: SkyScrollBarVertical {}

        RowLayout {
            id: msgRow
            width: parent.width
            spacing: 10

            Label {
                id: emojiLabel
                height: visible ? implicitHeight : 0
                verticalAlignment: Text.AlignVCenter
                font.pointSize: guiSettings.absScaledFont(6)
                font.family: UnicodeFonts.getEmojiFontFamily()
                text: emoji
                visible: emoji
            }

            AccessibleLabel {
                id: msgLabel
                Layout.fillWidth: true

                verticalAlignment: Text.AlignVCenter
                padding: 10
                textFormat: Text.RichText
                wrapMode: Text.Wrap

                onLinkActivated: (link) => msgDialog.linkActivated(link)
            }
        }
    }


    function show(msg) {
        msgLabel.text = msg
        open()
    }
}
