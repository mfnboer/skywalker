import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Dialog {
    property string authorName // empty, "me", "other"
    property string otherAuthorHandle
    property string mentionsName // empty, "me", "other"
    property string otherMentionsHandle
    property bool isTyping: false

    id: searchPostScope
    width: parent.width
    contentHeight: scopeGrid.height + (authorTypeaheadView.visible ? authorTypeaheadView.height : 0)
    topMargin: guiSettings.headerHeight
    title: qsTr("Post search scope")
    modal: true
    standardButtons: Dialog.Ok

    GridLayout {
        id: scopeGrid
        width: parent.width
        //height: everyoneButton.height + meButton.height + otherButton.height
        columns: 2

        AccessibleText {
            Layout.columnSpan: 2
            font.bold: true
            text: qsTr("From:")
        }

        ComboBox {
            id: authorComboBox
            background.implicitHeight: otherAuthorText.height
            textRole: "label"
            valueRole: "value"
            model: ListModel {
                ListElement { label: qsTr("Everyone"); value: "" }
                ListElement { label: qsTr("Me"); value: "me" }
                ListElement { label: qsTr("User"); value: "other" }
            }

            onCurrentValueChanged: {
                if (currentIndex >= 0)
                    authorName = currentValue
            }
        }

        SkyTextInput {
            id: otherAuthorText
            Layout.fillWidth: true
            svgIcon: svgOutline.user
            initialText: otherAuthorHandle
            placeholderText: qsTr("Enter user handle")
            enabled: authorName === "other"

            onDisplayTextChanged: {
                if (displayText !== initialText) {
                    searchPostScope.isTyping = true
                    authorTypeaheadView.textInput = otherAuthorText
                    authorTypeaheadSearchTimer.start()
                }
            }

            onEditingFinished: {
                searchPostScope.isTyping = false
                authorTypeaheadSearchTimer.stop()
                otherAuthorHandle = displayText
            }
        }

        AccessibleText {
            Layout.columnSpan: 2
            font.bold: true
            text: qsTr("Mentions:")
        }

        ComboBox {
            id: mentionsComboBox
            background.implicitHeight: otherMentionsText.height
            textRole: "label"
            valueRole: "value"
            model: ListModel {
                ListElement { label: qsTr(""); value: "" }
                ListElement { label: qsTr("Me"); value: "me" }
                ListElement { label: qsTr("User"); value: "other" }
            }

            onCurrentValueChanged: {
                if (currentIndex >= 0)
                    mentionsName = currentValue
            }
        }

        SkyTextInput {
            id: otherMentionsText
            Layout.fillWidth: true
            svgIcon: svgOutline.atSign
            initialText: otherMentionsHandle
            placeholderText: qsTr("Enter user handle")
            enabled: mentionsName === "other"

            onDisplayTextChanged: {
                if (displayText !== initialText) {
                    searchPostScope.isTyping = true
                    authorTypeaheadView.textInput = otherMentionsText
                    authorTypeaheadSearchTimer.start()
                }
            }

            onEditingFinished: {
                searchPostScope.isTyping = false
                authorTypeaheadSearchTimer.stop()
                otherMentionsHandle = displayText
            }
        }
    }

    SimpleAuthorListView {
        property var textInput

        id: authorTypeaheadView
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: scopeGrid.bottom
        height: 300
        model: searchUtils.authorTypeaheadList
        visible: searchPostScope.isTyping

        onAuthorClicked: (profile) => {
            textInput.text = profile.handle
            searchPostScope.isTyping = false
        }
    }

    Timer {
        id: authorTypeaheadSearchTimer
        interval: 500
        onTriggered: {
            const text = authorTypeaheadView.textInput.displayText

            if (text.length > 0)
                searchUtils.searchAuthorsTypeahead(text)
        }
    }

    SearchUtils {
        id: searchUtils
        skywalker: root.getSkywalker()
    }

    GuiSettings {
        id: guiSettings
    }

    function getAuthorName() {
        return authorName === "other" ? otherAuthorHandle : authorName
    }

    function getMentionsName() {
        return mentionsName === "other" ? otherMentionsHandle : mentionsName
    }

    Component.onCompleted: {
        authorComboBox.currentIndex = authorComboBox.indexOfValue(authorName)
        mentionsComboBox.currentIndex = mentionsComboBox.indexOfValue(mentionsName)
    }
}
