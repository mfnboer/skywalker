import QtQuick
import QtQuick.Controls
import skywalker

Text {
    required property string plainText
    property string elidedText
    property string ellipsisBackgroundColor: guiSettings.backgroundColor
    property bool mustClean: false
    property int initialShowMaxLineCount: maximumLineCount
    property int capLineCount: initialShowMaxLineCount

    id: theText
    height: textFormat === Text.RichText && elide === Text.ElideRight && wrapMode !== Text.NoWrap ?
                Math.min(contentHeight, capLineCount * fontMetrics.height) + topPadding + bottomPadding : undefined
    clip: true
    color: guiSettings.textColor
    text: plainText

    onPlainTextChanged: {
        determineTextFormat()
    }

    onWidthChanged: resetText()

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
        const graphemeInfo = unicodeFonts.getGraphemeInfo(elidedText)
        const newLength = Math.floor(graphemeInfo.length * ratio - 1)

        if (newLength < 1)
            return

        elidedText = graphemeInfo.sliced(elidedText, 0, newLength) + "…"
        setElidedText()
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
        elideRichText()

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
        visible: theText.height < theText.contentHeight
    }

    Label {
        id: showMoreLabel
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        leftPadding: theText.leftPadding
        background: Rectangle { color: ellipsisBackgroundColor }
        text: qsTr(`<a href="show"style="color: ${guiSettings.linkColor}">Show ${numLinesHidden()} lines more</a>`)
        visible: theText.height < theText.contentHeight && capLineCount < theText.maximumLineCount

        onLinkActivated:  capLineCount = theText.maximumLineCount
    }

    function numLinesHidden() {
        return Math.floor((contentHeight - height) / fontMetrics.height + 1)
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
