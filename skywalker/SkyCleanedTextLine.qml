import QtQuick
import skywalker

Text {
    required property string plainText
    readonly property bool mustClean: unicodeFonts.hasCombinedEmojis(plainText)
    property bool isCompleted: false

    id: theText
    color: GuiSettings.textColor
    textFormat: mustClean ? Text.RichText : Text.PlainText

    Accessible.role: Accessible.StaticText
    Accessible.name: plainText

    onPlainTextChanged: {
        if (isCompleted)
            setPlainText()
    }

    function elideText(txt) {
        if (elide !== Text.ElideRight)
            return txt

        if (contentWidth <= width)
            return txt

        if (txt.length < 2)
            return txt

        if (contentWidth <= 0)
            return txt

        const ratio = width / contentWidth
        const graphemeInfo = unicodeFonts.getGraphemeInfo(txt)
        const newLength = Math.floor(graphemeInfo.length * ratio - 1)

        if (newLength < 1)
            return txt

        return graphemeInfo.sliced(txt, 0, newLength) + "…"
    }

    function setPlainText() {
        text = mustClean ? unicodeFonts.toCleanedHtml(elideText(plainText)) : plainText
    }

    UnicodeFonts {
        id: unicodeFonts
    }

    Component.onCompleted: {
        isCompleted = true
        setPlainText()
    }
}
