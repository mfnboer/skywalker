import QtQuick
import QtQuick.Controls
import skywalker

SkyDialog {
    required property string joinUri

    id: dialog
    width: parent.width - 40
    contentHeight: joinLinkPreview.height
    standardButtons: Dialog.Close
    anchors.centerIn: parent

    Flickable {
        anchors.fill: parent
        clip: true
        contentWidth: parent.width
        contentHeight: joinLinkPreview.height
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds

        JoinLinkPreview {
            id: joinLinkPreview
            width: parent.width
            uri: joinUri

            onButtonClicked: dialog.reject()
        }
    }
}
