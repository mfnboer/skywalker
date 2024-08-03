import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Dialog {
    property string editWord
    property bool isTyping: false
    property bool isHashtag: false

    id: page
    width: parent.width
    contentHeight: textInput.height + (hashtagTypeaheadView.visible ? hashtagTypeaheadView.height : 0)
    topMargin: guiSettings.headerHeight
    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel
    Material.background: guiSettings.backgroundColor

    Accessible.role: Accessible.Dialog

    SkyTextInput {
        id: textInput
        width: parent.width
        svgIcon: svgOutline.mutedWords
        initialText: editWord
        placeholderText: qsTr("Word, phrase, or hashtag to mute")
        enabled: true

        onDisplayTextChanged: {
            page.isTyping = true

            if (displayText.length > 1 && unicodeFonts.isHashtag(displayText)) {
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

    HashtagListView {
        id: hashtagTypeaheadView
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: textInput.bottom
        height: 200
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

    UnicodeFonts {
        id: unicodeFonts
    }

    GuiSettings {
        id: guiSettings
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
