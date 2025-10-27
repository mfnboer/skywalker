import QtQuick
import QtQuick.Controls
import skywalker

// Some ugliness: this code depends on being a child of ComposePost
ScrollView {
    property int imgWidth: 240
    property bool requireAltText: false
    property var postUtils

    id: imageScroller
    height: visible && images.length > 0 ? 180 : 0
    anchors.topMargin: images.length > 0 ? 10 : 0
    horizontalPadding: 10
    contentWidth: imageRow.width
    contentHeight: height

    Row {
        id: imageRow
        width: images.length * imageScroller.imgWidth + (images.length - 1) * spacing
        spacing: 10

        Repeater {
            model: images

            Image {
                required property string modelData
                required property int index

                width: imageScroller.imgWidth
                height: imageScroller.height
                fillMode: Image.PreserveAspectCrop
                autoTransform: true
                source: modelData
                asynchronous: true

                Accessible.role: Accessible.StaticText
                Accessible.name: qsTr(`picture ${(index + 1)}: `) + (imageScroller.hasAltText(index) ? altTexts[index] : "no alt text")

                onStatusChanged: {
                    if (status === Image.Error){
                        skywalker.showStatusMessage(qsTr("Cannot load image"), QEnums.STATUS_LEVEL_ERROR);
                        imageScroller.removeImage(index)
                    }
                }

                SkyButton {
                    height: 34
                    flat: imageScroller.hasAltText(index)
                    text: imageScroller.hasAltText(index) ? qsTr("ALT") : qsTr("+ALT", "add alternative text button")
                    onClicked: imageScroller.editAltText(index)
                    Accessible.name: imageScroller.hasAltText(index) ? qsTr(`edit alt text for picture ${(index + 1)}`) : qsTr(`add alt text to picture ${(index + 1)}`)
                }

                SkyButton {
                    y: parent.height - height
                    height: 34
                    flat: imageScroller.hasMeme(index)
                    text: imageScroller.hasMeme(index) ? qsTr("MEME") : qsTr("+MEME")
                    onClicked: imageScroller.editMeme(index)
                    Accessible.name: imageScroller.hasMeme(index) ? qsTr(`edit meme text for picture ${(index + 1)}`) : qsTr(`add meme text to picture ${(index + 1)}`)
                }

                SvgButton {
                    id: closeButton
                    x: parent.width - width
                    width: 34
                    height: width
                    svg: SvgOutline.close
                    accessibleName: qsTr(`remove picture ${(index + 1)}`)
                    onClicked: imageScroller.removeImage(index)
                }

                SvgButton {
                    x: closeButton.x - width
                    width: 34
                    height: width
                    svg: SvgOutline.edit
                    accessibleName: qsTr(`edit picture ${(index + 1)}`)
                    onClicked: imageScroller.editImage(index)
                }

                SvgButton {
                    x: parent.width - width
                    y: parent.height - height
                    width: 34
                    height: width
                    svg: SvgOutline.share
                    accessibleName: qsTr(`share picture ${(index + 1)} to other app`)
                    onClicked: postUtils.sharePhotoToApp(modelData)
                }

                SkyLabel {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    backgroundColor: guiSettings.errorColor
                    horizontalAlignment: Text.AlignHCenter
                    color: "white"
                    text: "ALT text missing"
                    visible: requireAltText && !hasAltText(index)

                    Accessible.role: Accessible.StaticText
                    Accessible.name: qsTr(`picture ${(index + 1)}: `) + text
                }
            }
        }
    }

    function removeImage(index) {
        postUtils.dropPhoto(images[index])
        altTexts.splice(index, 1)
        images.splice(index, 1)
        memeTopTexts.splice(index, 1)
        memeBottomTexts.splice(index, 1)
    }

    function hasAltText(index) {
        if (index >= altTexts.length)
            return false

        return altTexts[index].length > 0
    }

    function hasMeme(index) {
        if (index >= memeTopTexts.length || index >= memeBottomTexts.length)
            return false

        return memeTopTexts[index].length > 0 || memeBottomTexts[index].length > 0
    }

    function editAltText(index) {
        let component = guiSettings.createComponent("AltTextEditor.qml")
        let altPage = component.createObject(page, {
            imgSource: images[index],
            text: altTexts[index] })
        altPage.onAltTextChanged.connect((text) => {
            altTexts[index] = text
            root.popStack()
        })
        root.pushStack(altPage)
    }

    function makeMemeAltText(topText, bottomText) {
        let altText = topText

        if (bottomText) {
            if (altText)
                altText += '\n'

            altText += bottomText
        }

        return altText
    }

    function editMeme(index) {
        let component = guiSettings.createComponent("MemeEditor.qml")
        let memePage = component.createObject(page, {
            imgSource: images[index],
            memeTopText: memeTopTexts[index],
            memeBottomText: memeBottomTexts[index]
        })
        memePage.onMeme.connect((topText, bottomText) => {
            memeTopTexts[index] = topText
            memeBottomTexts[index] = bottomText
            altTexts[index] = makeMemeAltText(topText, bottomText)
            root.popStack()
        })
        memePage.onCancel.connect(() => root.popStack())
        root.pushStack(memePage)
    }

    function editImage(index) {
        let component = guiSettings.createComponent("EditImage.qml")
        let editPage = component.createObject(page, {
            imgSource: images[index]
        })
        editPage.onDone.connect((newImgSource) => {
            images[index] = newImgSource
            root.popStack()
        })
        editPage.onCancel.connect(() => root.popStack())
        root.pushStack(editPage)
    }
}
