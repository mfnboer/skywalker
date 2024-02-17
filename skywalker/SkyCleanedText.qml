import QtQuick
import QtQuick.Controls
import skywalker

Text {
    required property string plainText
    property string elidedText
    property string ellipsisBackgroundColor: guiSettings.backgroundColor
    property bool mustClean: false

    id: theText
    clip: true
    text: plainText

    onPlainTextChanged: determineTextFormat()
    onWidthChanged: resetText()
    onHeightChanged: capLinesRichText()

    onTextChanged: {
        elideRichText()
        capLinesRichText()
    }

    Accessible.role: Accessible.StaticText
    Accessible.name: plainText

    function isRichText() {
        return textFormat === Text.RichText
    }

    function setElidedText() {
        if (mustClean)
            text = unicodeFonts.toCleanedHtml(elidedText)
        else
            text = elidedText
    }

    function elideRichText() {
        if (!isRichText())
            return

        if (elide !== Text.ElideRight)
            return

        if (wrapMode !== Text.NoWrap)
            return

        if (contentWidth <= width)
            return

        if (elidedText.length < 2)
            return

        if (contentWidth <= 0)
            return

        const ratio = width / contentWidth
        const newLength = elidedText.length * ratio - 1

        if (newLength < 1)
            return

        // TODO: should slice graphemes
        elidedText = elidedText.slice(0, newLength) + "…"
        setElidedText()
    }

    function capLinesRichText() {
        if (!isRichText())
            return

        if (elide !== Text.ElideRight)
            return

        if (wrapMode === Text.NoWrap)
            return

        if (ellipsis.visible)
            return

        if (fontMetrics.height <= 0)
            return

        const numLines = Math.floor(height / fontMetrics.height)

        if (numLines <= maximumLineCount)
            return

        height = height * (maximumLineCount / numLines)
        ellipsis.visible = true
    }

    function resetText() {
        if (!isRichText())
            return

        if (elidedText !== plainText) {
            elidedText = plainText
            setElidedText()
        }

        elideRichText()
    }

    function determineTextFormat() {
        text = plainText
        height = undefined

        if (isRichText())
            return

        if (unicodeFonts.hasCombinedEmojis(plainText)) {
            textFormat = Text.RichText
            mustClean = true
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
        visible: false
    }

    FontMetrics {
        id: fontMetrics
        font: theText.font
    }

    UnicodeFonts {
        id: unicodeFonts
    }

    GuiSettings {
        id: guiSettings
    }
}
