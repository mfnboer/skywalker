import QtQuick
import skywalker

Text {
    required property string plainText
    property string elidedText
    property bool mustClean: false

    id: theText
    text: plainText

    onWidthChanged: resetText()
    onTextChanged: elideRichText()

    // TODO: num lines = HEIGHT / FONT HEIGHT (approx)
    onHeightChanged: console.debug("FONT HEIGHT:", fontMetrics.height, "FONT PT:", font.pointSize, "HEIGHT:", height, plainText)

    Accessible.role: Accessible.StaticText
    Accessible.name: plainText

    function elideRichText() {
        if (!mustClean)
            return

        if (elide !== Text.ElideRight)
            return

        if (wrapMode !== Text.NoWrap)
            return // TODO

        if (contentWidth <= width)
            return

        if (elidedText.length < 2)
            return

        elidedText = elidedText.slice(0, elidedText.length - 2) + "…"
        text = unicodeFonts.toCleanedHtml(elidedText)
    }

    function resetText() {
        if (!mustClean)
            return

        if (elidedText !== plainText) {
            elidedText = plainText
            text = unicodeFonts.toCleanedHtml(elidedText)
        }

        elideRichText()
    }

    FontMetrics {
        id: fontMetrics
        font: theText.font
    }

    UnicodeFonts {
        id: unicodeFonts
    }

    Component.onCompleted: {
        if (unicodeFonts.hasCombinedEmojis(plainText)) {
            textFormat = Text.RichText
            mustClean = true

        }

        resetText()
    }
}
