import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Page {
    required property var skywalker
    required property int purpose // QEnums::ListPurpose
    property listview list

    signal closed

    id: editListPage
    width: parent.width

    header: SimpleHeader {
        text: list.isNull() ? qsTr(`New ${(listTypeName())}`) : qsTr(`Edit ${(listTypeName())}`)
        onBack: editListPage.closed()
    }

    footer: Rectangle {
        id: pageFooter
        width: editListPage.width
        height: getFooterHeight()
        z: guiSettings.footerZLevel
        color: guiSettings.footerColor
        visible: descriptionField.activeFocus

        function getFooterHeight() {
            return guiSettings.footerHeight
        }

        ProgressBar {
            id: textLengthBar
            anchors.left: parent.left
            anchors.right: parent.right
            from: 0
            to: Math.max(descriptionField.maxLength, descriptionField.graphemeLength)
            value: descriptionField.graphemeLength

            contentItem: Rectangle {
                width: textLengthBar.visualPosition * parent.width
                height: parent.height
                color: descriptionField.graphemeLength <= descriptionField.maxLength ? guiSettings.buttonColor : guiSettings.errorColor
            }
        }

        Text {
            y: 10
            anchors.rightMargin: 10
            anchors.right: parent.right
            color: descriptionField.graphemeLength <= descriptionField.maxLength ? guiSettings.textColor : guiSettings.errorColor
            text: descriptionField.maxLength - descriptionField.graphemeLength
        }
    }

    Connections {
        target: Qt.inputMethod

        // Resize the footer when the Android virtual keyboard is shown
        function onKeyboardRectangleChanged() {
            if (Qt.inputMethod.keyboardRectangle.y > 0) {
                const keyboardY = Qt.inputMethod.keyboardRectangle.y  / Screen.devicePixelRatio
                pageFooter.height = pageFooter.getFooterHeight() + (parent.height - keyboardY)
            }
            else {
                pageFooter.height = pageFooter.getFooterHeight()
            }
        }
    }

    Flickable {
        id: flick
        width: parent.width
        anchors.fill: parent
        clip: true
        contentWidth: pageColumn.width
        contentHeight: pageColumn.y + descriptionRect.y + descriptionField.y + descriptionField.height
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds
        onHeightChanged: ensureVisible(descriptionField.cursorRectangle)

        function ensureVisible(cursor) {
            let cursorY = cursor.y + pageColumn.y + descriptionRect.y + descriptionField.y

            if (contentY >= cursorY)
                contentY = cursorY;
            else if (contentY + height <= cursorY + cursor.height)
                contentY = cursorY + cursor.height - height;
        }

        ColumnLayout {
            id: pageColumn
            x: 10
            y: 10
            width: editListPage.width - 2 * x

            Text {
                Layout.fillWidth: true
                font.bold: true
                color: guiSettings.textColor
                text: qsTr("List Avatar")
            }

            FeedAvatar {
                width: 100
                avatarUrl: list.avatar
            }

            Text {
                Layout.fillWidth: true
                topPadding: 10
                font.bold: true
                color: guiSettings.textColor
                text: qsTr("List Name")
            }

            SkyTextInput {
                id: nameField
                Layout.fillWidth: true
                focus: true
                initialText: list.name
                placeholderText: qsTr("Name your list")
                maximumLength: 64
            }

            Text {
                Layout.fillWidth: true
                topPadding: 10
                font.bold: true
                color: guiSettings.textColor
                text: qsTr("List Description")
            }

            Rectangle {
                id: descriptionRect
                Layout.fillWidth: true
                height: descriptionField.height
                radius: 10
                border.width: 1
                border.color: guiSettings.borderColor
                color: guiSettings.backgroundColor

                SkyFormattedTextEdit {
                    readonly property int maxLength: 300

                    id: descriptionField
                    width: parent.width
                    topPadding: 10
                    bottomPadding: 10
                    parentPage: editListPage
                    parentFlick: flick
                    placeholderText: qsTr("Describe your list")
                    initialText: list.description
                }
            }
        }
    }

    GuiSettings {
        id: guiSettings
    }

    function listTypeName() {
        switch (purpose) {
        case QEnums.LIST_PURPOSE_MOD:
            return qsTr("Moderation List")
        case QEnums.LIST_PURPOSE_CURATE:
            return qsTr("User List")
        default:
            return qsTr("List")
        }
    }
}
