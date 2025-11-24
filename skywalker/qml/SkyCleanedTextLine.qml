import QtQuick
import skywalker

Text {
    required property string plainText
    property bool mustClean: false
    property bool isCompleted: false
    property bool inWidthChanged: false

    // When textFormat = Text.RichText then Text.contentWidth gives the full width
    // of the field. advanceWidth seems to work better.
    readonly property real advanceWidth: textMetrics.advanceWidth

    id: theText
    color: guiSettings.textColor
    textFormat: mustClean ? Text.RichText : Text.PlainText

    Accessible.role: Accessible.StaticText
    Accessible.name: plainText

    onPlainTextChanged: {
        if (isCompleted)
            setPlainText()
    }

    onWidthChanged: {
        if (inWidthChanged)
            return

        if (isCompleted) {
            inWidthChanged = true
            setPlainText()
            inWidthChanged = false
        }
    }

    function elideText() {
        if (elide !== Text.ElideRight)
            return

        if (contentWidth <= width)
            return

        if (text.length < 2)
            return

        if (contentWidth <= 0)
            return

        const ratio = width / contentWidth
        const graphemeInfo = UnicodeFonts.getGraphemeInfo(plainText)
        const newLength = Math.floor(graphemeInfo.length * ratio - 1)

        if (newLength < 1)
            return

        const elidedText = graphemeInfo.sliced(plainText, 0, newLength) + "…"
        text = UnicodeFonts.toCleanedHtml(elidedText)
    }

    function setPlainText() {
        mustClean = UnicodeFonts.hasCombinedEmojis(plainText)

        if (mustClean) {
            text = UnicodeFonts.toCleanedHtml(plainText)
            elideText()
        }
        else {
            text = plainText
        }
    }

    Loader {
        readonly property real advanceWidth: active ? Math.min(item.advanceWidth, theText.width) : theText.contentWidth

        id: textMetrics
        active: theText.textFormat == Text.RichText

        sourceComponent: TextMetrics {
            font.family: theText.fontInfo.family
            font.bold: theText.fontInfo.bold
            font.italic: theText.fontInfo.italic
            font.pointSize: theText.fontInfo.pointSize
            text: theText.plainText
        }
    }

    Component.onCompleted: {
        isCompleted = true
        setPlainText()
    }
}
