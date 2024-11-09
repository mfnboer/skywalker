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
    Material.background: GuiSettings.backgroundColor

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

            AccessibleCheckBox {
                checked: suggestive
                text: qsTr("Sexually suggestive")
                onCheckedChanged: suggestive = checked
            }
            AccessibleCheckBox {
                checked: nudity
                text: qsTr("Non-sexual Nudity")
                onCheckedChanged: nudity = checked
            }
            AccessibleCheckBox {
                checked: porn
                text: qsTr("Adult Content, e.g. pornography")
                onCheckedChanged: porn = checked
            }
            AccessibleCheckBox {
                checked: gore
                text: qsTr("Graphic Media, e.g. violent/bloody")
                onCheckedChanged: gore = checked
            }
        }
    }
}
