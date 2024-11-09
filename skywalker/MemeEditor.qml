import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyPage {
    required property string imgSource
    required property string memeTopText
    required property string memeBottomText
    readonly property int margin: 10

    signal cancel
    signal meme(string topText, string bottomText)

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
            svg: SvgOutline.check
            accessibleName: qsTr("add meme")
            onClicked: page.meme(memeMaker.topText, memeMaker.bottomText)
        }
    }

    Flickable {
        anchors.fill: parent
        clip: true
        contentWidth: parent.width
        contentHeight: memeImage.y + memeImage.height
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds

        SkyTextInput {
            id: topText
            x: margin
            width: parent.width - 2 * margin
            initialText: memeTopText
            placeholderText: qsTr("Top text")

            onDisplayTextChanged: memeMaker.topText = displayText
        }

        SkyTextInput {
            id: bottomText
            anchors.top: topText.bottom
            anchors.topMargin: 10
            x: margin
            width: parent.width - 2 * margin
            initialText: memeBottomText
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
    }

    MemeMaker {
        id: memeMaker
    }


    Component.onCompleted: {
        if (!memeMaker.setOrigImage(imgSource)) {
            root.getSkywalker().showStatusMessage(qsTr("Failed to load image"))
            cancel()
            return
        }

        memeMaker.topText = memeTopText
        memeMaker.bottomText = memeBottomText
        topText.setFocus()
    }
}
