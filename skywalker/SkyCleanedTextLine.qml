import QtQuick
import skywalker

Text {
    required property string plainText
    property bool mustClean: false
    property bool isCompleted: false
    property bool inWidthChanged: false

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


    Component.onCompleted: {
        isCompleted = true
        setPlainText()
    }
}
