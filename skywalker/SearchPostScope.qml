import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Dialog {
    property string authorName // "all", "me", "other"
    property string otherAuthorHandle
    property string mentionsName // "all", "me", "other"
    property string otherMentionsHandle
    property date sinceDate
    property bool setSince: false
    property date untilDate
    property bool setUntil: false
    property string language
    property bool isTyping: false
    property bool initialized: false

    id: searchPostScope
    width: parent.width
    contentHeight: Math.max(scopeGrid.height, (authorTypeaheadView.visible ? authorTypeaheadView.y + authorTypeaheadView.height : 0))
    topMargin: guiSettings.headerHeight
    modal: true
    standardButtons: Dialog.Ok
    Material.background: guiSettings.backgroundColor

    GridLayout {
        id: scopeGrid
        width: parent.width
        columns: 2

        AccessibleText {
            font.bold: true
            text: qsTr("From:")
        }

        EditComboBox {
            id: authorComboBox
            Layout.fillWidth: true
            textRole: "label"
            valueRole: "value"
            model: ListModel {
                ListElement { label: qsTr("Everyone"); value: "all" }
                ListElement { label: qsTr("Me"); value: "me" }
                ListElement { label: qsTr("Enter user handle"); value: "other" }
            }
            editableIndex: 2
            initialEditValue: otherAuthorHandle

            onCurrentValueChanged: {
                if (initialized && currentValue !== undefined)
                    authorName = currentValue
            }

            onInputTextChanged: (textInput) => {
                if (textInput.displayText !== initialEditValue) {
                    searchPostScope.isTyping = true
                    authorTypeaheadView.textInputCombo = authorComboBox
                    authorTypeaheadSearchTimer.start()
                    otherAuthorHandle = textInput.displayText
                }
            }

            onEditingFinished: (text) => {
                searchPostScope.isTyping = false
                authorTypeaheadSearchTimer.stop()
                otherAuthorHandle = text
            }
        }

        AccessibleText {
            font.bold: true
            text: qsTr("Mentions:")
        }

        EditComboBox {
            id: mentionsComboBox
            Layout.fillWidth: true
            textRole: "label"
            valueRole: "value"
            model: ListModel {
                ListElement { label: qsTr("-"); value: "all" }
                ListElement { label: qsTr("Me"); value: "me" }
                ListElement { label: qsTr("Enter user handle"); value: "other" }
            }
            editableIndex: 2
            initialEditValue: otherMentionsHandle

            onCurrentValueChanged: {
                if (initialized && currentValue !== undefined)
                    mentionsName = currentValue
            }

            onInputTextChanged: (textInput) => {
                if (textInput.displayText !== initialEditValue) {
                    searchPostScope.isTyping = true
                    authorTypeaheadView.textInputCombo = mentionsComboBox
                    authorTypeaheadSearchTimer.start()
                    otherMentionsHandle = textInput.displayText
                }
            }

            onEditingFinished: (text) => {
                searchPostScope.isTyping = false
                authorTypeaheadSearchTimer.stop()
                otherMentionsHandle = text
            }
        }

        AccessibleText {
            font.bold: true
            text: qsTr("Since:")
        }

        SkyTextInput {
            id: sinceText
            Layout.fillWidth: true
            placeholderText: qsTr("Date")
            text: setSince ? sinceDate.toLocaleDateString(Qt.locale(), Locale.ShortFormat) : ""

            MouseArea {
                anchors.fill: parent
                onClicked: selectDate(sinceText)
            }

            SvgButton {
                width: 40
                height: width
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                svg: SvgOutline.cancel
                iconColor: guiSettings.textColor
                Material.background: "transparent"
                accessibleName: qsTr("clear since date")
                visible: setSince
                onClicked: {
                    setSince = false
                    sinceText.clear()
                }
            }
        }

        AccessibleText {
            font.bold: true
            text: qsTr("Until:")
        }

        SkyTextInput {
            id: untilText
            Layout.fillWidth: true
            placeholderText: qsTr("Date")
            text: setUntil ? untilDate.toLocaleDateString(Qt.locale(), Locale.ShortFormat) : ""

            MouseArea {
                anchors.fill: parent
                onClicked: selectDate(untilText)
            }

            SvgButton {
                width: 40
                height: width
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                svg: SvgOutline.cancel
                iconColor: guiSettings.textColor
                Material.background: "transparent"
                accessibleName: qsTr("clear until date")
                visible: setUntil
                onClicked: {
                    setUntil = false
                    untilText.clear()
                }
            }
        }

        AccessibleText {
            font.bold: true
            text: qsTr("Language:")
        }

        LanguageComboBox {
            id: languageComboBox
            Layout.fillWidth: true
            background.implicitHeight: untilText.height
            radius: 5
            color: guiSettings.textColor
            borderColor: guiSettings.borderColor
            borderWidth: 1
            allLanguages: languageUtils.languages
            usedLanguages: languageUtils.usedLanguages
            addAnyLanguage: true
            initialLanguage: language

            onActivated: (index) => {
                if (!initialized)
                    return

                if (index > 0)
                    language = valueAt(index)
                else
                    language = ""
            }
        }
    }

    SimpleAuthorListView {
        property var textInputCombo: authorComboBox

        id: authorTypeaheadView
        anchors.left: parent.left
        anchors.right: parent.right
        y: textInputCombo.y + textInputCombo.height
        height: 300
        model: searchUtils.authorTypeaheadList
        visible: searchPostScope.isTyping

        onAuthorClicked: (profile) => {
            textInputCombo.contentItem.text = profile.handle
            searchPostScope.isTyping = false
        }
    }

    Timer {
        id: authorTypeaheadSearchTimer
        interval: 500
        onTriggered: {
            const text = authorTypeaheadView.textInputCombo.contentItem.displayText

            if (text.length > 0)
                searchUtils.searchAuthorsTypeahead(text)
        }
    }

    SearchUtils {
        id: searchUtils
        skywalker: root.getSkywalker()
    }

    LanguageUtils {
        id: languageUtils
        skywalker: root.getSkywalker()
    }


    function selectDate(textInput) {
        let component = Qt.createComponent("DatePicker.qml")
        let datePicker = component.createObject(parent, { selectedDate: (textInput === sinceText) ? sinceDate : untilDate })
        datePicker.onRejected.connect(() => datePicker.destroy())

        datePicker.onAccepted.connect(() => {
            if (textInput === sinceText) {
                sinceDate = datePicker.selectedDate
                setSince = true
            }
            else {
                untilDate = datePicker.selectedDate
                setUntil = true
            }

            datePicker.destroy()
        })

        datePicker.open()
    }

    function getUserName(userName, otherUserHandle) {
        if (userName === "all")
            return ""

        if (userName === "other") {
            if (otherUserHandle.startsWith('@'))
                return otherAuthorHandle.slice(1)

            return otherUserHandle
        }

        return userName
    }

    function getAuthorName() {
        return getUserName(authorName, otherAuthorHandle)
    }

    function getMentionsName() {
        return getUserName(mentionsName, otherMentionsHandle)
    }

    function startOfToday() {
        let d = new Date()
        d.setHours(0, 0, 0, 0)
        return d
    }

    Component.onCompleted: {
        if (!setSince)
            sinceDate = startOfToday()

        if (!setUntil) {
            untilDate = startOfToday()
            untilDate.setDate(untilDate.getDate() + 1)
        }

        authorComboBox.setSelection(authorName)
        mentionsComboBox.setSelection(mentionsName)

        initialized = true
    }
}
