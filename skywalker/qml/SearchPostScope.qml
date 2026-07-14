import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyDialog {
    required property var skywalker
    property var userSettings: skywalker.getUserSettings()
    readonly property string userDid: skywalker.getUserDid()
    property string authorName // "all", "me", "other"
    property string otherAuthorHandles // space separated handles
    property string mentionsName // "all", "me", "other"
    property string otherMentionsHandles // space separated handles
    property date sinceDate
    property bool setSince: false
    property date untilDate
    property bool setUntil: false
    property string language
    readonly property bool adultContent: skywalker.getContentFilter().getAdultContent()
    property int overrideAdultVisibility: userSettings.getSearchAdultOverrideVisibility(userDid)
    property bool isTyping: false
    property bool initialized: false

    id: searchPostScope
    width: parent.width
    contentHeight: Math.max(scopeGrid.height, (authorTypeaheadView.visible ? authorTypeaheadView.y + authorTypeaheadView.height : 0))
    topMargin: guiSettings.headerHeight
    standardButtons: Dialog.Ok

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
                ListElement { label: qsTr("Anyone"); value: "all" }
                ListElement { label: qsTr("People you follow"); value: "following" }
                ListElement { label: qsTr("Me"); value: "me" }
                ListElement { label: qsTr("Enter user handle(s)"); value: "other" }
            }
            editableIndex: 3
            initialEditValue: otherAuthorHandles

            onCurrentValueChanged: {
                if (initialized && currentValue !== undefined)
                    authorName = currentValue
            }

            onInputTextChanged: (textInput) => {
                if (textInput.displayText !== initialEditValue) {
                    searchPostScope.isTyping = true
                    authorTypeaheadView.textInputCombo = authorComboBox
                    authorTypeaheadView.startSearch()
                    otherAuthorHandles = textInput.displayText
                }
            }

            onEditingFinished: (text) => {
                searchPostScope.isTyping = false
                authorTypeaheadView.stopSearch()
                otherAuthorHandles = text
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
                ListElement { label: qsTr("Enter user handle(s)"); value: "other" }
            }
            editableIndex: 2
            initialEditValue: otherMentionsHandles

            onCurrentValueChanged: {
                if (initialized && currentValue !== undefined)
                    mentionsName = currentValue
            }

            onInputTextChanged: (textInput) => {
                if (textInput.displayText !== initialEditValue) {
                    searchPostScope.isTyping = true
                    authorTypeaheadView.textInputCombo = mentionsComboBox
                    authorTypeaheadView.startSearch()
                    otherMentionsHandles = textInput.displayText
                }
            }

            onEditingFinished: (text) => {
                searchPostScope.isTyping = false
                authorTypeaheadView.stopSearch()
                otherMentionsHandles = text
            }
        }

        AccessibleText {
            font.bold: true
            text: qsTr("Since:")
        }

        SkyTextInput {
            id: sinceText
            Layout.fillWidth: true
            svgIcon: SvgOutline.date
            placeholderText: qsTr("Date")
            text: setSince ? sinceDate.toLocaleDateString(Qt.locale(), Locale.ShortFormat) : ""

            SkyMouseArea {
                anchors.fill: parent
                onClicked: selectDate(sinceText)
            }

            SvgPlainButton {
                width: 40
                height: width
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                svg: SvgOutline.cancel
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
            svgIcon: SvgOutline.date
            placeholderText: qsTr("Date")
            text: setUntil ? untilDate.toLocaleDateString(Qt.locale(), Locale.ShortFormat) : ""

            SkyMouseArea {
                anchors.fill: parent
                onClicked: selectDate(untilText)
            }

            SvgPlainButton {
                width: 40
                height: width
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                svg: SvgOutline.cancel
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
            backgroundColor: guiSettings.textInputBackgroundColor
            borderColor: guiSettings.buttonColor
            borderWidth: languageComboBox.activeFocus ? 1 : 0
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

        AccessibleText {
            font.bold: true
            text: qsTr("Adult:")
            visible: adultContent
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: -1
            visible: adultContent

            SkyRadioButton {
                Layout.fillWidth: true
                checked: overrideAdultVisibility === QEnums.CONTENT_VISIBILITY_SHOW
                horizontalAlignment: Qt.AlignHCenter
                text: qsTr("On")
                onCheckedChanged: {
                    if (checked)
                        overrideAdultVisibility = QEnums.CONTENT_VISIBILITY_SHOW
                }
            }
            SkyRadioButton {
                Layout.fillWidth: true
                checked: overrideAdultVisibility === QEnums.CONTENT_VISIBILITY_WARN_MEDIA
                horizontalAlignment: Qt.AlignHCenter
                text: qsTr("Warn")
                onCheckedChanged: {
                    if (checked)
                        overrideAdultVisibility = QEnums.CONTENT_VISIBILITY_WARN_MEDIA
                }
            }
            SkyRadioButton {
                Layout.fillWidth: true
                checked: overrideAdultVisibility === QEnums.CONTENT_VISIBILITY_HIDE_MEDIA
                horizontalAlignment: Qt.AlignHCenter
                text: qsTr("Hide")
                onCheckedChanged: {
                    if (checked)
                        overrideAdultVisibility = QEnums.CONTENT_VISIBILITY_HIDE_MEDIA
                }
            }
        }
    }

    SimpleAuthorTypeaheadListView {
        property var textInputCombo: authorComboBox

        id: authorTypeaheadView
        anchors.left: parent.left
        anchors.right: parent.right
        y: textInputCombo.y + textInputCombo.height
        height: 300
        searchText: textInputCombo.contentItem.cursorWord
        visible: searchPostScope.isTyping

        onAuthorClicked: (profile) => {
            textInputCombo.contentItem.replaceCursorWord(profile.handle)
            searchPostScope.isTyping = false
        }
    }

    LanguageUtils {
        id: languageUtils
        skywalker: root.getSkywalker()
    }


    function selectDate(textInput) {
        let component = guiSettings.createComponent("DatePicker.qml")
        let datePicker = component.createObject(parent, { selectedDate: (textInput === sinceText) ? sinceDate : untilDate })
        datePicker.onRejected.connect(() => datePicker.close())

        datePicker.onAccepted.connect(() => {
            if (textInput === sinceText) {
                sinceDate = datePicker.selectedDate
                setSince = true
            }
            else {
                untilDate = datePicker.selectedDate
                setUntil = true
            }

            datePicker.close()
        })

        datePicker.open()
    }

    function getUserNames(userName, otherUserHandle) {
        if (userName === "all" || userName === "following")
            return []

        if (userName === "other") {
            const s = otherUserHandle.replace(/@/g, "")
            return s.trim().split(/\s+/)
        }

        return [userName]
    }

    function getFollowing() {
        return (authorName === "following")
    }

    function getAuthorNames() {
        return getUserNames(authorName, otherAuthorHandles)
    }

    function getMentionNames() {
        return getUserNames(mentionsName, otherMentionsHandles)
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
