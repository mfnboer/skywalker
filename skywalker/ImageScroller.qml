import QtQuick
import QtQuick.Controls
import skywalker

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

                Accessible.role: Accessible.StaticText
                Accessible.name: qsTr(`picture ${(index + 1)}: `) + (imageScroller.hasAltText(index) ? altTexts[index] : "no alt text")

                onStatusChanged: {
                    if (status === Image.Error){
                        statusPopup.show(qsTr("Cannot load image"), QEnums.STATUS_LEVEL_ERROR);
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

                SvgButton {
                    x: parent.width - width
                    width: 34
                    height: width
                    svg: svgOutline.close
                    accessibleName: qsTr(`remove picture ${(index + 1)}`)
                    onClicked: imageScroller.removeImage(index)
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

    GuiSettings {
        id: guiSettings
    }

    function removeImage(index) {
        postUtils.dropPhoto(images[index])
        altTexts.splice(index, 1)
        images.splice(index, 1)
    }

    function hasAltText(index) {
        if (index >= altTexts.length)
            return false

        return altTexts[index].length > 0
    }

    function editAltText(index) {
        let component = Qt.createComponent("AltTextEditor.qml")
        let altPage = component.createObject(page, {
                imgSource: images[index],
                text: altTexts[index] })
        altPage.onAltTextChanged.connect((text) => {
                altTexts[index] = text
                root.popStack()
        })
        root.pushStack(altPage)
    }
}
