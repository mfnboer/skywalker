import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    required property string link
    property string name

    id: page
    width: parent.width
    contentHeight: linkColumn.height
    topMargin: guiSettings.headerHeight
    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel
    Material.background: guiSettings.backgroundColor

    ColumnLayout {
        id: linkColumn
        width: parent.width
        spacing: 5

        AccessibleText {
            font.bold: true
            text: qsTr("Display text")
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: nameField.height
            radius: 5
            border.width: 1
            border.color: guiSettings.borderColor
            color: "transparent"

            SkyTextEdit {
                id: nameField
                width: parent.width
                topPadding: 10
                bottomPadding: 10
                focus: true
                initialText: page.name
                placeholderText: qsTr("Display text for link")
                inputMethodHints: Qt.ImhNoAutoUppercase
                singleLine: true
            }
        }

        AccessibleText {
            topPadding: 10
            font.bold: true
            text: qsTr("Link")
        }

        AccessibleText {
            Layout.fillWidth: true
            color: guiSettings.linkColor
            wrapMode: Text.Wrap
            text: page.link
        }
    }

    function getName() {
        return nameField.text.trim()
    }
}
