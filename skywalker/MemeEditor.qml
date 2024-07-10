import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Page {
    required property string imgSource
    readonly property int margin: 10

    signal cancel
    signal meme(string source)

    id: page
    width: parent.width
    topPadding: 10
    bottomPadding: 10

    header: SimpleHeader {
        text: qsTr("Meme maker")
        backIsCancel: true
        onBack: page.cancel()

        SvgButton {
            id: okButton
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            svg: svgOutline.check
            accessibleName: qsTr("add meme")
            onClicked: {
                const source = memeMaker.memeImgSource
                memeMaker.releaseMemeOwnership()
                page.meme(source)
            }
        }
    }

    SkyTextInput {
        id: topText
        x: margin
        width: parent.width - 2 * margin
        placeholderText: qsTr("Top text")

        onDisplayTextChanged: memeMaker.topText = displayText
    }

    SkyTextInput {
        id: bottomText
        anchors.top: topText.bottom
        anchors.topMargin: 10
        x: margin
        width: parent.width - 2 * margin
        placeholderText: qsTr("Bottom text")

        onDisplayTextChanged: memeMaker.bottomText = displayText
    }

    Image {
        id: memeImage
        x: margin
        width: parent.width - 2 * margin
        anchors.top: bottomText.bottom
        anchors.topMargin: 10
        fillMode: Image.PreserveAspectFit
        autoTransform: true
        source: memeMaker.memeImgSource
    }

    MemeMaker {
        id: memeMaker
        screenWidth: page.width - 2 * margin
    }

    GuiSettings {
        id: guiSettings
    }

    Component.onCompleted: {
        // TODO: handle error
        memeMaker.setOrigImage(imgSource)
    }
}
