import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Text {
    required property string plainText
    property string elidedText
    property bool showEllipsis: true
    property string ellipsisBackgroundColor: guiSettings.backgroundColor
    property bool mustClean: false
    property int initialShowMaxLineCount: maximumLineCount
    property int capLineCount: initialShowMaxLineCount
    readonly property bool mustElideRich: elide === Text.ElideRight && textFormat === Text.RichText
    property bool isCompleted: false
    property bool inWidthChanged: false

    // When textFormat = Text.RichText then Text.contentWidth gives the full width
    // of the field. advanceWidth seems to work better.
    readonly property real advanceWidth: textMetrics.advanceWidth

    id: theText
    height: mustElideRich && wrapMode !== Text.NoWrap ?
        Math.min(contentHeight, capLineCount * fontMetrics.height) + topPadding + bottomPadding : undefined
    Layout.maximumHeight: mustElideRich && wrapMode !== Text.NoWrap ?
        capLineCount * fontMetrics.height + topPadding + bottomPadding : -1
    clip: true
    font.pointSize: guiSettings.scaledFont(1)
    color: guiSettings.textColor

    onPlainTextChanged: {
        if (isCompleted) {
            determineTextFormat()
        }
    }

    onWidthChanged: {
        if (inWidthChanged)
            return

        inWidthChanged = true
        resetText()
        inWidthChanged = false
    }

    Accessible.role: Accessible.StaticText
    Accessible.name: plainText

    function setElidedText() {
        if (mustClean)
            text = UnicodeFonts.toCleanedHtml(elidedText)
        else
            text = elidedText
    }

    function elideRichText() {
        if (contentWidth <= width)
            return

        if (elidedText.length < 2)
            return

        if (contentWidth <= 0)
            return

        const ratio = width / contentWidth
        const graphemeInfo = UnicodeFonts.getGraphemeInfo(elidedText)
        const newLength = Math.floor(graphemeInfo.length * ratio - 1)

        if (newLength < 1)
            return

        elidedText = graphemeInfo.sliced(elidedText, 0, newLength) + "…"
    }

    function resetText() {
        if (!mustElideRich)
            return

        text = plainText
        elidedText = plainText
        elideRichText()
        setElidedText()
    }

    function determineTextFormat() {
        text = plainText

        if (mustElideRich) {
            elidedText = plainText
            elideRichText()
            setElidedText()
            return
        }

        if (textFormat === Text.RichText)
            return

        if (UnicodeFonts.hasCombinedEmojis(plainText)) {
            mustClean = true
            textFormat = Text.RichText
            resetText()
        }
    }

    Loader {
        active: theText.height > 0 && theText.height < theText.contentHeight && showEllipsis
        anchors.right: parent.right
        anchors.rightMargin: parent.rightPadding
        anchors.bottom: parent.bottom
        visible: status == Loader.Ready

        sourceComponent: Label {
            id: ellipsis
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            leftPadding: 10
            font: theText.font
            color: theText.color
            background: Rectangle { color: ellipsisBackgroundColor }
            text: "…"
        }
    }

    Loader {
        active: theText.height > 0 && theText.height < theText.contentHeight && capLineCount < theText.maximumLineCount
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        visible: status == Loader.Ready

        sourceComponent: AccessibleLabel {
            id: showMoreLabel
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            leftPadding: theText.leftPadding
            bottomPadding: 10
            background: Rectangle { color: ellipsisBackgroundColor }
            wrapMode: Text.Wrap
            text: qsTr(`<a href="show" style="color: ${guiSettings.linkColor}">Show ${numLinesHidden()} lines more</a>`)

            onLinkActivated:  capLineCount = theText.maximumLineCount
        }
    }

    function numLinesHidden() {
        return Math.floor((contentHeight - height) / fontMetrics.height + 1)
    }

    FontMetrics {
        id: fontMetrics
        font: theText.font
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
        determineTextFormat()
    }
}
