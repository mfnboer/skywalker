import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Dialog {
    property string focusHashtag
    property bool isTyping: false

    id: page
    width: parent.width
    contentHeight: textInput.height + (hashtagTypeaheadView.visible ? hashtagTypeaheadView.height : 0)
    topMargin: guiSettings.headerHeight
    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel

    SkyTextInput {
        id: textInput
        width: parent.width
        svgIcon: svgOutline.hashtag
        initialText: focusHashtag
        placeholderText: qsTr("Hashtag for focus")
        validator: RegularExpressionValidator { regularExpression: /[^ ]+/ }
        enabled: true

        onDisplayTextChanged: {
            page.isTyping = true
            hashtagTypeaheadSearchTimer.start()
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
        visible: page.isTyping

        onHashtagClicked: (tag) => {
            textInput.text = `#${tag}`
            page.isTyping = false
        }
    }

    Timer {
        id: hashtagTypeaheadSearchTimer
        interval: 500
        onTriggered: {
            let text = textInput.displayText

            if (text.startsWith('#'))
                text = text.slice(1) // strip #-symbol

            if (text.length > 0)
                searchUtils.searchHashtagsTypeahead(text)
        }
    }

    SearchUtils {
        id: searchUtils
        skywalker: root.getSkywalker()
    }

    GuiSettings {
        id: guiSettings
    }

    function getText() {
        let text = textInput.text.trim()

        if (text.startsWith('#'))
            text = text.slice(1) // strip #-symbol

        return text
    }

    function show() {
        open()
    }

    Component.onCompleted: {
        textInput.setFocus()
    }
}
