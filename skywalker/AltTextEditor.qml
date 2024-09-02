import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyPage {
    required property string imgSource
    property alias text: altText.text
    property var skywalker: root.getSkywalker()
    readonly property int margin: 10

    signal altTextChanged(string text)

    id: page
    width: parent.width
    Material.background: guiSettings.backgroundColor
    topPadding: 10
    bottomPadding: 10

    header: SimpleButtonHeader {
        title: qsTr("ALT text")
        buttonSvg: svgOutline.check
        onButtonClicked: altTextChanged(page.text)
    }

    // Needed for SkyFormattedTextEdit
    footer: Rectangle {
        height: 0
        color: "transparent"
    }

    Flickable {
        id: flick
        anchors.fill: parent
        clip: true
        contentWidth: parent.width
        contentHeight: altImage.y + altImage.height
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds
        onHeightChanged: altText.ensureVisible(altText.cursorRectangle)

        SkyFormattedTextEdit {
            id: altText
            width: parent.width
            leftPadding: page.margin
            rightPadding: page.margin
            parentPage: page
            parentFlick: flick
            placeholderText: qsTr("Help users with visual impairments")
        }

        Image {
            id: altImage
            anchors.leftMargin: page.margin
            anchors.left: parent.left
            anchors.topMargin: 10
            anchors.top: altText.bottom
            width: 240
            height: 180
            fillMode: Image.PreserveAspectCrop
            autoTransform: true
            source: page.imgSource
        }
    }

    VirtualKeyboardPageResizer {
        id: virtualKeyboardPageResizer
    }

    Component.onCompleted: {
        // Save the full page height now. Later when the Android keyboard pops up,
        // the page height sometimes changes by itself, but not always...
        virtualKeyboardPageResizer.fullPageHeight = parent.height

        altText.forceActiveFocus()
    }
}
