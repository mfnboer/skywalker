import QtQuick
import QtQuick.Controls
import skywalker

VideoThumbnail {
    id: videoAttachement

    function remove() {
        postUtils.dropVideo(video)
        video = ""
        altText = ""
    }

    function hasAltText() {
        return Boolean(altText)
    }

    SkyButton {
        height: 34
        flat: videoAttachement.hasAltText()
        text: videoAttachement.hasAltText() ? qsTr("ALT") : qsTr("+ALT", "add alternative text button")
        onClicked: videoAttachement.editAltText()
        Accessible.name: imageScroller.hasAltText(index) ? qsTr(`edit alt text for picture ${(index + 1)}`) : qsTr(`add alt text to picture ${(index + 1)}`)
    }

    SvgButton {
        x: parent.width - width
        width: 34
        height: width
        svg: svgOutline.close
        accessibleName: qsTr("remove video")
        onClicked: videoAttachement.remove()
    }
}
