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
    readonly property bool mustElideRich: elide === Text.ElideRight && textFormat === Text.RichText
    property bool isCompleted: false
    property bool inWidthChanged: false

    id: theText
    height: mustElideRich && wrapMode !== Text.NoWrap ?
                Math.min(contentHeight, capLineCount * fontMetrics.height) + topPadding + bottomPadding : undefined
    clip: true
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
        active: theText.height > 0 && theText.height < theText.contentHeight
        anchors.right: parent.right
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

        sourceComponent: Label {
            id: showMoreLabel
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            leftPadding: theText.leftPadding
            bottomPadding: 10
            background: Rectangle { color: ellipsisBackgroundColor }
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

    Component.onCompleted: {
        isCompleted = true
        determineTextFormat()
    }
}
