import QtQuick
import QtQuick.Controls
import skywalker

Dialog {
    property string prefix

    x: (parent.width - width) / 2
    width: 250
    title: qsTr("Prefix")
    contentHeight: exampleText.y + exampleText.height
    topMargin: guiSettings.headerHeight
    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel
    Material.background: guiSettings.backgroundColor

    Rectangle {
        width: parent.width
        height: textInput.height
        radius: 5
        border.width: textInput.activeFocus ? 1 : 0
        border.color: guiSettings.buttonColor
        color: guiSettings.textInputBackgroundColor

        SkyTextEdit {
            id: textInput
            width: parent.width
            topPadding: 10
            bottomPadding: 10
            maxLength: 5
            strictMax: true
            singleLine: true
            inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
            placeholderText: qsTr("<none>")
            initialText: prefix
        }

        SvgPlainButton {
            id: clearButton
            anchors.right: parent.right
            imageMargin: 8
            y: parent.y - parent.padding
            width: height
            height: parent.height
            svg: SvgOutline.close
            accessibleName: qsTr("reset prefix")
            onClicked: textInput.text = UnicodeFonts.THREAD_SYMBOL
        }
    }
    AccessibleText {
        id: exampleText
        y: textInput.y + textInput.height + 10
        width: parent.width
        textFormat: Text.RichText
        text: UnicodeFonts.toCleanedHtml(qsTr(`Example: ${getPrefix()}1/2`))
    }


    function getPrefix() {
        return textInput.text
    }
}
