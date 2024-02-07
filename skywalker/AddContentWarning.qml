import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Dialog {
    required property bool suggestive
    required property bool nudity
    required property bool porn
    required property bool gore

    id: contentWarningDialog
    width: parent.width
    contentHeight: warningColumn.height
    title: qsTr("Adult content warning")
    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel
    anchors.centerIn: parent

    Flickable {
        anchors.fill: parent
        clip: true
        contentWidth: parent.width
        contentHeight: warningColumn.height
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds

        Column {
            id: warningColumn
            width: parent.width

            CheckBox {
                checked: suggestive
                text: qsTr("Sexually suggestive")
                onCheckedChanged: suggestive = checked
            }
            CheckBox {
                checked: nudity
                text: qsTr("Nudity")
                onCheckedChanged: nudity = checked
            }
            CheckBox {
                checked: porn
                text: qsTr("Pornography / Sexually explicit")
                onCheckedChanged: porn = checked
            }
            CheckBox {
                checked: gore
                text: qsTr("Violent / Bloody")
                onCheckedChanged: gore = checked
            }
        }
    }
}
