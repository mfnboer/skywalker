import QtQuick
import skywalker

SkyPage {
    required property string imgSource

    signal cancel
    signal done(string newImgSource)

    id: page
    width: parent.width

    header: SimpleHeader {
        text: qsTr("Edit Image")
        backIsCancel: true
        onBack: cancel()

        SvgPlainButton {
            anchors.right: parent.right
            anchors.top: parent.top
            svg: SvgOutline.check
            accessibleName: qsTr("editing finished")
            onClicked: transformImage()
        }
    }

    footer: DeadFooterMargin {}

    Image {
        property bool sidesSwapped: false

        id: img
        y: (page.height - buttonRow.height - height) / 2
        width: page.width
        height: page.height - buttonRow.height
        fillMode: Image.PreserveAspectFit
        autoTransform: true
        source: imgSource
        scale: !sidesSwapped ? 1.0 : (paintedWidth > height ? height / paintedWidth : 1.0)
        transform: []
    }

    Row {
        id: buttonRow
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
        topPadding: 10

        SvgButton {
            svg: SvgOutline.mirrorHor
            accessibleName: qsTr("mirror horizontally")
            onClicked: page.mirror()
        }

        SvgButton {
            svg: SvgOutline.rotate90ccw
            accessibleName: qsTr("rotate 90 degrees counter clockwise")
            onClicked: page.rotate()
        }
    }

    ImageUtils {
        id: imageUtils
    }

    Component {
        id: mirrorComponent

        Scale {
            origin.x: img.width / 2
            xScale: -1
        }
    }

    Component {
        id: rotateComponent

        Rotation {
            origin.x: img.width / 2
            origin.y: img.height / 2
            angle: -90
        }
    }

    function mirror() {
        img.transform.push(mirrorComponent.createObject())
    }

    function rotate() {
        img.transform.push(rotateComponent.createObject())
        img.sidesSwapped = !img.sidesSwapped
    }

    function transformImage() {
        let transformations = []

        for (const t of img.transform) {
            if (t instanceof Scale)
                transformations.push(QEnums.IMAGE_TRANSFORM_MIRROR)
            else if (t instanceof Rotation)
                transformations.push(QEnums.IMAGE_TRANSFORM_ROTATE)
            else
                console.warn("Unknown transform:", t)
        }

        const newImgSource = imageUtils.transformImage(imgSource, transformations)
        done(newImgSource)
    }
}
