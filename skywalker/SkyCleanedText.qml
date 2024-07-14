import QtQuick
import QtQuick.Controls
import skywalker

Text {
    required property string plainText
    property string elidedText
    property string ellipsisBackgroundColor: guiSettings.backgroundColor
    property bool mustClean: false
    property int intialShowMaxLineCount: maximumLineCount
    property bool isCapped: false
    property bool isCapping: false
    property int heightBeforeCap
    property int lineCountBeforeCap
    property int capLineCount: intialShowMaxLineCount

    id: theText
    clip: true
    color: guiSettings.textColor
    text: plainText

    onPlainTextChanged: determineTextFormat()
    onWidthChanged: resetText()

    onHeightChanged: {
        if (!isCapping)
            capLinesRichText()
    }

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
        const graphemeInfo = unicodeFonts.getGraphemeInfo(elidedText)
        const newLength = Math.floor(graphemeInfo.length * ratio - 1)

        if (newLength < 1)
            return

        elidedText = graphemeInfo.sliced(elidedText, 0, newLength) + "…"
        setElidedText()
    }

    function capLinesRichText() {
        // TODO: capping on new lines
        // if (!isRichText())
        //     return

        // if (elide !== Text.ElideRight)
        //     return

        // if (wrapMode === Text.NoWrap)
        //     return

        // if (isCapped)
        //     return

        // if (fontMetrics.height <= 0)
        //     return

        // const numLines = Math.floor(height / fontMetrics.height)

        // if (numLines <= capLineCount)
        //     return

        // isCapping = true
        // lineCountBeforeCap = numLines
        // heightBeforeCap = height
        // height = height * (capLineCount / numLines)
        // isCapped = true
        // isCapping = false
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
        //height = undefined
        isCapped = false
        capLinesRichText()

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
        visible: isCapped
    }

    Label {
        id: showMoreLabel
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        leftPadding: theText.leftPadding
        background: Rectangle { color: ellipsisBackgroundColor }
        text: qsTr(`<a href="show"style="color: ${guiSettings.linkColor}">Show ${(lineCountBeforeCap - capLineCount + 1)} lines more</a>`)
        visible: isCapped && capLineCount < theText.maximumLineCount

        onLinkActivated: {
            isCapped = false
            capLineCount = theText.maximumLineCount
            theText.height = heightBeforeCap
        }
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
