import QtQuick
import QtQuick.Controls

SkyPage {
    property alias text: detailsText.text
    property var skywalker: root.getSkywalker()
    readonly property int margin: 10

    signal detailsChanged(string text)

    id: page
    width: parent.width
    topPadding: 10
    bottomPadding: 10
    Material.background: guiSettings.backgroundColor

    header: SimpleButtonHeader {
        title: qsTr("Report details")
        buttonSvg: svgOutline.check
        enabled: !detailsText.maxGraphemeLengthExceeded()
        onButtonClicked: detailsChanged(page.text)
    }

    footer: Rectangle {
        id: pageFooter
        width: page.width
        height: guiSettings.footerHeight
        z: guiSettings.footerZLevel
        color: guiSettings.footerColor

        TextLengthBar {
            textField: detailsText
        }

        TextLengthCounter {
            y: 10
            anchors.rightMargin: page.margin
            anchors.right: parent.right
            textField: detailsText
        }
    }

    Flickable {
        id: flick
        anchors.fill: parent
        clip: true
        contentWidth: parent.width
        contentHeight: detailsText.y + detailsText.height
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds
        onHeightChanged: detailsText.ensureVisible(detailsText.cursorRectangle)

        SkyFormattedTextEdit {
            id: detailsText
            width: parent.width
            leftPadding: page.margin
            rightPadding: page.margin
            parentPage: page
            parentFlick: flick
            placeholderText: qsTr("Reason or any other details")
            maxLength: 2000
        }
    }

    VirtualKeyboardPageResizer {
        id: virtualKeyboardPageResizer
    }

    GuiSettings {
        id: guiSettings
    }

    Component.onCompleted: {
        // Save the full page height now. Later when the Android keyboard pops up,
        // the page height sometimes changes by itself, but not always...
        virtualKeyboardPageResizer.fullPageHeight = parent.height

        detailsText.forceActiveFocus()
    }
}
