import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

TextEdit {
    required property var parentPage
    required property var parentFlick
    property string initialText: ""
    property string placeholderText
    property int graphemeLength: 0
    property int maxLength: -1
    property bool enableLinkShortening: true

    id: editText
    width: parentPage.width
    leftPadding: 10
    rightPadding: 10
    activeFocusOnTab: true
    textFormat: TextEdit.PlainText
    wrapMode: TextEdit.Wrap
    font.pointSize: guiSettings.scaledFont(9/8)
    color: guiSettings.textColor
    selectionColor: guiSettings.selectionColor
    clip: true
    text: initialText

    Accessible.role: Accessible.EditableText
    Accessible.name: text ? text : qsTr(`Enter ${placeholderText}`)
    Accessible.description: Accessible.name
    Accessible.editable: true
    Accessible.multiLine: true

    onCursorRectangleChanged: {
        let editMentionY = postUtils.editMentionCursorY
        let editTagY = postUtils.editTagCursorY
        let cursorY = cursorRectangle.y

        if (postUtils.editMention.length > 0 && editMentionY != cursorY)
            postUtils.editMention = ""

        if (postUtils.editTag.length > 0 && editTagY != cursorY)
            postUtils.editTag = ""

        parentFlick.ensureVisible(cursorRectangle)
    }

    onTextChanged: {
        highlightFacets()
        updateGraphemeLength()
    }

    onPreeditTextChanged: updateGraphemeLength()

    onMaxLengthChanged: {
        postUtils.setHighlightDocument(editText.textDocument, guiSettings.linkColor,
                                       editText.maxLength, guiSettings.textLengthExceededColor)
    }

    function highlightFacets() {
        postUtils.extractMentionsAndLinks(editText.text,
                editText.preeditText, cursorPosition)
    }

    function updateGraphemeLength() {
        graphemeLength = unicodeFonts.graphemeLength(editText.text) +
                unicodeFonts.graphemeLength(preeditText) -
                (enableLinkShortening ? postUtils.getLinkShorteningReduction() : 0)
    }

    Text {
        anchors.fill: parent
        leftPadding: editText.leftPadding
        rightPadding: editText.rightPadding
        topPadding: editText.topPadding
        bottomPadding: editText.bottomPadding
        font.pointSize: editText.font.pointSize
        color: guiSettings.placeholderTextColor
        text: placeholderText
        visible: editText.graphemeLength === 0
    }

    Timer {
        id: authorTypeaheadSearchTimer
        interval: 500
        onTriggered: {
            if (postUtils.editMention.length > 0)
                searchUtils.searchAuthorsTypeahead(postUtils.editMention, 10)
        }
    }

    Timer {
        id: hashtagTypeaheadSearchTimer
        interval: 500
        onTriggered: {
            if (postUtils.editTag.length > 0)
                searchUtils.searchHashtagsTypeahead(postUtils.editTag, 10)
        }
    }

    SearchUtils {
        id: searchUtils
        skywalker: parentPage.skywalker

        Component.onDestruction: {
            // The destuctor of SearchUtils is called too late by the QML engine
            // Remove models now before the Skywalker object is destroyed.
            searchUtils.removeModels()
        }
    }

    PostUtils {
        property double editMentionCursorY: 0
        property double editTagCursorY: 0

        id: postUtils
        skywalker: parentPage.skywalker

        onEditMentionChanged: {
            console.debug(editMention)
            editMentionCursorY = editText.cursorRectangle.y
            authorTypeaheadSearchTimer.start()
        }

        onEditTagChanged: {
            console.debug(editTag)
            editTagCursorY = editText.cursorRectangle.y
            hashtagTypeaheadSearchTimer.start()
        }
    }

    UnicodeFonts {
        id: unicodeFonts
    }

    GuiSettings {
        id: guiSettings
    }

    function createAuthorTypeaheadView() {
        let component = Qt.createComponent("AuthorTypeaheadView.qml")
        let page = component.createObject(parentPage, {
                parentPage: parentPage,
                editText: editText,
                searchUtils: searchUtils,
                postUtils: postUtils
            })
    }

    function createHashtagTypeaheadView() {
        let component = Qt.createComponent("HashtagTypeaheadView.qml")
        let page = component.createObject(parentPage, {
                parentPage: parentPage,
                editText: editText,
                searchUtils: searchUtils,
                postUtils: postUtils
            })
    }

    Component.onCompleted: {
        createAuthorTypeaheadView()
        createHashtagTypeaheadView()
        postUtils.setHighlightDocument(editText.textDocument, guiSettings.linkColor,
                                       editText.maxLength, guiSettings.textLengthExceededColor)
    }
}

