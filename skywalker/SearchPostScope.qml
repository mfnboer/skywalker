import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Dialog {
    property string userName // empty, "me", "other"
    property string otherUserHandle
    property bool isTyping: false

    id: searchPostScope
    width: parent.width
    contentHeight: scopeGrid.height + hastagTypeaheadView.height
    title: qsTr("Posts from:")
    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel
    anchors.centerIn: parent

    Accessible.role: Accessible.Dialog
    Accessible.name: title

    GridLayout {
        id: scopeGrid
        width: parent.width
        height: everyoneButton.height + meButton.height + otherButton.height
        columns: 2

        RadioButton {
            id: everyoneButton
            Layout.columnSpan: 2
            checked: !userName
            text: qsTr("Everyone")

            onCheckedChanged: {
                if (checked)
                    userName = ""
            }
        }
        RadioButton {
            id: meButton
            Layout.columnSpan: 2
            checked: userName === "me"
            text: qsTr("Me")

            onCheckedChanged: {
                if (checked)
                    userName = "me"
            }
        }
        RadioButton {
            id: otherButton
            text: qsTr("User")
            checked: userName === "other"

            onCheckedChanged: {
                if (checked)
                    userName = "other"
            }
        }
        SkyTextInput {
            id: otherUserText
            Layout.fillWidth: true
            svgIcon: svgOutline.user
            initialText: otherUserHandle
            placeholderText: qsTr("Enter user handle")
            enabled: otherButton.checked

            onTextEdited: {
                searchPostScope.isTyping = true
                authorTypeaheadSearchTimer.start()
            }

            onEditingFinished: {
                searchPostScope.isTyping = false
                authorTypeaheadSearchTimer.stop()
                userName = displayText
            }
        }
    }

    SimpleAuthorListView {
        id: hastagTypeaheadView
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: scopeGrid.bottom
        height: 300
        model: searchUtils.authorTypeaheadList
        visible: searchPostScope.isTyping

        onAuthorClicked: (profile) => {
            otherUserText.text = profile.handle
            searchPostScope.isTyping = false
        }
    }

    Timer {
        id: authorTypeaheadSearchTimer
        interval: 500
        onTriggered: {
            const text = otherUserText.displayText

            if (text.length > 0)
                searchUtils.searchAuthorsTypeahead(text)
        }
    }

    SearchUtils {
        id: searchUtils
        skywalker: root.getSkywalker()
    }

    function getUserName() {
        return userName === "other" ? otherUserHandle : userName
    }
}
