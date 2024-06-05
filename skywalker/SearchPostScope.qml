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
    property bool isTyping: false
    property bool initialized: false

    id: searchPostScope
    width: parent.width
    contentHeight: Math.max(scopeGrid.height, (authorTypeaheadView.visible ? authorTypeaheadView.y + authorTypeaheadView.height : 0))
    topMargin: guiSettings.headerHeight
    modal: true
    standardButtons: Dialog.Ok

    GridLayout {
        id: scopeGrid
        width: parent.width
        columns: 2

        AccessibleText {
            font.bold: true
            text: qsTr("From:")
        }

        ComboBox {
            id: authorComboBox
            Layout.fillWidth: true
            background.implicitHeight: otherAuthorText.height
            textRole: "label"
            valueRole: "value"
            model: ListModel {
                ListElement { label: qsTr("Everyone"); value: "all" }
                ListElement { label: qsTr("Me"); value: "me" }
                ListElement { label: qsTr("User"); value: "other" }
            }

            onCurrentValueChanged: {
                if (initialized)
                    authorName = currentValue
            }
        }

        Rectangle {
            width: 1
            height: 1
            color: "transparent"
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
            font.bold: true
            text: qsTr("Mentions:")
        }

        ComboBox {
            id: mentionsComboBox
            Layout.fillWidth: true
            background.implicitHeight: otherMentionsText.height
            textRole: "label"
            valueRole: "value"
            model: ListModel {
                ListElement { label: qsTr("-"); value: "all" }
                ListElement { label: qsTr("Me"); value: "me" }
                ListElement { label: qsTr("User"); value: "other" }
            }

            onCurrentValueChanged: {
                if (initialized)
                    mentionsName = currentValue
            }
        }

        Rectangle {
            width: 1
            height: 1
            color: "transparent"
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
                svg: svgOutline.cancel
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
                svg: svgOutline.cancel
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
    }

    SimpleAuthorListView {
        property var textInput

        id: authorTypeaheadView
        anchors.left: parent.left
        anchors.right: parent.right
        y: textInput.y + textInput.height
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

        return userName === "other" ? otherUserHandle : userName
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

        authorComboBox.currentIndex = authorComboBox.indexOfValue(authorName)
        mentionsComboBox.currentIndex = mentionsComboBox.indexOfValue(mentionsName)
        initialized = true
    }
}
