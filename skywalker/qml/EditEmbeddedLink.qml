import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyDialog {
    required property int linkType // QEnums::linkType
    required property string link
    property string name
    property string error
    property bool canAddLinkCard: false
    property bool addLinkCard: false

    id: page
    width: parent.width
    contentHeight: linkColumn.height
    topMargin: guiSettings.headerHeight
    standardButtons: Dialog.Ok | Dialog.Cancel

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
            radius: guiSettings.radius
            border.width: nameField.activeFocus ? 1 : 0
            border.color: guiSettings.buttonColor
            color: guiSettings.textInputBackgroundColor

            SkyTextEdit {
                id: nameField
                width: parent.width
                topPadding: 10
                bottomPadding: 10
                focus: true
                initialText: page.name
                placeholderText: qsTr(`Display text for ${getLinkTypeName()}`)
                inputMethodHints: Qt.ImhNoAutoUppercase
                singleLine: true
            }
        }

        AccessibleText {
            Layout.fillWidth: true
            color: guiSettings.errorColor
            wrapMode: Text.Wrap
            text: "⚠️ " + error
            visible: Boolean(error)
        }

        AccessibleText {
            topPadding: 10
            font.bold: true
            text: getLinkTypeName()
        }

        AccessibleText {
            Layout.fillWidth: true
            color: guiSettings.linkColor
            wrapMode: Text.Wrap
            text: page.link
        }

        AccessibleCheckBox {
            leftPadding: 0
            topPadding: 10
            Layout.fillWidth: true
            checked: addLinkCard
            text: qsTr("Add link card")
            visible: canAddLinkCard
            onCheckedChanged: addLinkCard = checked
        }
    }

    function getName() {
        return nameField.text.trim()
    }

    function getLinkTypeName() {
        switch (linkType) {
        case QEnums.LINK_TYPE_WEB:
            return qsTr("Link")
        case QEnums.LINK_TYPE_MENTION:
            return qsTr("Mention")
        }

        return "unknown link type"
    }
}
