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

    Accessible.role: Accessible.Dialog
    Accessible.name: title

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
                text: qsTr("Nudity")
                onCheckedChanged: nudity = checked
            }
            AccessibleCheckBox {
                checked: porn
                text: qsTr("Pornography / Sexually explicit")
                onCheckedChanged: porn = checked
                Accessible.onPressAction: toggle()
            }
            AccessibleCheckBox {
                checked: gore
                text: qsTr("Violent / Bloody")
                onCheckedChanged: gore = checked
            }
        }
    }
}
