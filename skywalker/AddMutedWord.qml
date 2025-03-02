import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Dialog {
    property string editWord
    property date expiresAt
    property bool excludeFollows: false
    property bool isTyping: false
    property bool isHashtag: false
    property date nullDate

    id: page
    width: parent.width
    contentHeight: excludeFollowsSwitch.y + excludeFollowsSwitch.height
    topMargin: guiSettings.headerHeight
    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel
    Material.background: guiSettings.backgroundColor

    SkyTextInput {
        id: textInput
        width: parent.width
        svgIcon: SvgOutline.mutedWords
        initialText: editWord
        placeholderText: qsTr("Word, phrase, or hashtag to mute")
        enabled: true

        onDisplayTextChanged: {
            page.isTyping = true

            if (displayText.length > 1 && UnicodeFonts.isHashtag(displayText)) {
                page.isHashtag = true
                hashtagTypeaheadSearchTimer.start()
            }
            else {
                page.isHashtag = false
                hashtagTypeaheadSearchTimer.stop()
            }
        }

        onEditingFinished: {
            page.isTyping = false
            hashtagTypeaheadSearchTimer.stop()
        }
    }

    AccessibleText {
        id: validityHeader
        anchors.top: textInput.bottom
        anchors.topMargin: 10
        width: parent.width
        font.bold: true
        text: qsTr('Duration:')
    }

    ButtonGroup {
        id: durationGroup
    }

    GridLayout {
        id: durationGrid
        anchors.top: validityHeader.bottom
        width: parent.width
        rowSpacing: 0
        columns: 2

        SkyRoundRadioButton {
            id: foreverButton
            text: qsTr("Forever")
            ButtonGroup.group: durationGroup
            onCheckedChanged: {
                if (checked)
                    expiresAt = nullDate
            }
        }
        SkyRoundRadioButton {
            text: qsTr("24 hours")
            ButtonGroup.group: durationGroup
            onCheckedChanged: {
                if (checked)
                    setExpiresAtDays(1)
            }
        }
        SkyRoundRadioButton {
            text: qsTr("7 days")
            ButtonGroup.group: durationGroup
            onCheckedChanged: {
                if (checked)
                    setExpiresAtDays(7)
            }
        }
        SkyRoundRadioButton {
            text: qsTr("30 days")
            ButtonGroup.group: durationGroup
            onCheckedChanged: {
                if (checked)
                    setExpiresAtDays(30)
            }
        }

        RowLayout {
            Layout.columnSpan: 2

            SkyRoundRadioButton {
                id: untilButton
                text: qsTr("Until:")
                ButtonGroup.group: durationGroup
            }
            SkyTextInput {
                id: untilIntput
                Layout.fillWidth: true
                svgIcon: SvgOutline.date
                placeholderText: qsTr("Date, time")
                text: enabled ? expiresAt.toLocaleString(Qt.locale(), Locale.ShortFormat) : ""
                enabled: untilButton.checked

                MouseArea {
                    anchors.fill: parent
                    onClicked: selectExpiresDate()
                }
            }
        }
    }

    AccessibleSwitch {
        id: excludeFollowsSwitch
        anchors.top: durationGrid.bottom
        anchors.topMargin: 10
        width: parent.width
        text: qsTr("Exclude users you follow")
        checked: excludeFollows
        onCheckedChanged: excludeFollows = checked
    }

    HashtagListView {
        id: hashtagTypeaheadView
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: textInput.bottom
        anchors.bottom: excludeFollowsSwitch.bottom
        model: searchUtils.hashtagTypeaheadList
        visible: page.isTyping && page.isHashtag

        onHashtagClicked: (tag) => {
            textInput.text = `#${tag}`
            page.isTyping = false
        }
    }

    Timer {
        id: hashtagTypeaheadSearchTimer
        interval: 500
        onTriggered: {
            const text = textInput.displayText

            if (text.length > 1)
                searchUtils.searchHashtagsTypeahead(text.slice(1)) // strip #-symbol
        }
    }

    SearchUtils {
        id: searchUtils
        skywalker: root.getSkywalker()
    }

    function setExpiresAtDays(days) {
        expiresAt = new Date()
        expiresAt.setDate(expiresAt.getDate() + days)
    }

    function selectExpiresDate() {
        if (isNaN(expiresAt.getTime()))
            setExpiresAtDays(1)

        let component = guiSettings.createComponent("DatePicker.qml")
        let datePicker = component.createObject(parent, { selectedDate: expiresAt, enableTime: true })
        datePicker.onRejected.connect(() => datePicker.destroy())

        datePicker.onAccepted.connect(() => {
            expiresAt = datePicker.selectedDate
            datePicker.destroy()
        })

        datePicker.open()
    }

    function getText() {
        return textInput.text.trim()
    }

    function show() {
        open()
    }

    Component.onCompleted: {
        if (!isNaN(expiresAt.getTime()))
            untilButton.checked = true
        else
            foreverButton.checked = true

        textInput.setFocus()
    }
}
