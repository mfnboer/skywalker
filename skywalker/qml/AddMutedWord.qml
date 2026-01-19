import QtQuick
import QtQuick.Controls
import skywalker

Dialog {
    property string editWord
    property alias expiresAt: durationInput.expiresAt
    property bool excludeFollows: false
    property bool isTyping: false
    property bool isHashtag: false

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
        placeholderText: qsTr("Word, phrase, hashtag, cashtag")
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
        id: muteDomain
        anchors.top: textInput.bottom
        anchors.topMargin: 10
        width: parent.width
        font.pointSize: guiSettings.scaledFont(7/8)
        color: Material.color(Material.Grey)
        text: qsTr(`Mute links with domain ${textInput.displayText}`)
        visible: linkUtils.isDomain(textInput.displayText)
    }

    AccessibleText {
        id: validityHeader
        anchors.top: muteDomain.visible ? muteDomain.bottom : textInput.bottom
        anchors.topMargin: 10
        width: parent.width
        font.bold: true
        text: qsTr('Duration:')
    }

    DurationInput {
        id: durationInput
        anchors.top: validityHeader.bottom
        width: parent.width
    }

    AccessibleCheckBox {
        id: excludeFollowsSwitch
        anchors.top: durationInput.bottom
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

    LinkUtils {
        id: linkUtils
        skywalker: root.getSkywalker()
    }

    function getText() {
        return textInput.text.trim()
    }

    function show() {
        open()
    }

    Component.onCompleted: {
        textInput.setFocus()
    }
}
