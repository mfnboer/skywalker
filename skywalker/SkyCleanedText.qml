import QtQuick
import QtQuick.Controls
import skywalker

Text {
    required property string plainText
    property string elidedText
    property string ellipsisBackgroundColor: GuiSettings.backgroundColor
    property bool mustClean: false
    property int initialShowMaxLineCount: maximumLineCount
    property int capLineCount: initialShowMaxLineCount
    readonly property bool mustElideRich: elide === Text.ElideRight && wrapMode === Text.NoWrap && textFormat === Text.RichText

    id: theText
    height: textFormat === Text.RichText && elide === Text.ElideRight && wrapMode !== Text.NoWrap ?
                Math.min(contentHeight, capLineCount * fontMetrics.height) + topPadding + bottomPadding : undefined
    clip: true
    color: GuiSettings.textColor
    text: mustElideRich ? plainText : textMetrics.elidedText

    onPlainTextChanged: {
        determineTextFormat()
    }

    onWidthChanged: resetText()

    Accessible.role: Accessible.StaticText
    Accessible.name: plainText

    function setElidedText() {
        if (mustClean)
            text = unicodeFonts.toCleanedHtml(elidedText)
        else
            text = elidedText
    }

    function elideRichText() {
        if (!mustElideRich)
            return

        if (contentWidth <= width)
            return

        if (elidedText.length < 2)
            return

        if (contentWidth <= 0)
            return

        const ratio = width / contentWidth
        const graphemeInfo = unicodeFonts.getGraphemeInfo(elidedText)
        const newLength = Math.floor(graphemeInfo.length * ratio - 1)

        if (newLength < 1)
            return

        elidedText = graphemeInfo.sliced(elidedText, 0, newLength) + "…"
        setElidedText()
    }

    function resetText() {
        if (!mustElideRich)
            return

        if (elidedText !== plainText) {
            elidedText = plainText
            setElidedText()
        }

        elideRichText()
    }

    function determineTextFormat() {
        if (mustElideRich) {
            text = plainText
            elidedText = plainText
            elideRichText()
        }

        if (textFormat === Text.RichText) {
            return
        }

        if (unicodeFonts.hasCombinedEmojis(plainText)) {
            mustClean = true
            textFormat = Text.RichText
            resetText()
        }
    }

    Label {
        id: ellipsis
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        leftPadding: 10
        font: theText.font
        color: theText.color
        background: Rectangle { color: ellipsisBackgroundColor }
        text: "…"
        visible: theText.height < theText.contentHeight
    }

    Label {
        id: showMoreLabel
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        leftPadding: theText.leftPadding
        background: Rectangle { color: ellipsisBackgroundColor }
        text: qsTr(`<a href="show"style="color: ${GuiSettings.linkColor}">Show ${numLinesHidden()} lines more</a>`)
        visible: theText.height < theText.contentHeight && capLineCount < theText.maximumLineCount

        onLinkActivated:  capLineCount = theText.maximumLineCount
    }

    function numLinesHidden() {
        return Math.floor((contentHeight - height) / fontMetrics.height + 1)
    }

    TextMetrics {
        id: textMetrics
        font: theText.font
        elide:  (theText.wrapMode === Text.NoWrap && !mustElideRich) ? theText.elide : Text.ElideNone
        elideWidth: theText.width
        text: theText.plainText
    }

    FontMetrics {
        id: fontMetrics
        font: theText.font
    }

    UnicodeFonts {
        id: unicodeFonts
    }


    Component.onCompleted: {
        determineTextFormat()
    }
}
