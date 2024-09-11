import QtQuick
import QtQuick.Controls
import skywalker

VideoThumbnail {
    property bool requireAltText: false
    required property int videoEndMs

    signal edit

    id: videoAttachement

    SkyButton {
        height: 34
        flat: videoAttachement.hasAltText()
        text: videoAttachement.hasAltText() ? qsTr("ALT") : qsTr("+ALT", "add alternative text button")
        onClicked: videoAttachement.editAltText()
        Accessible.name: videoAttachement.hasAltText(index) ? qsTr(`edit alt text for picture ${(index + 1)}`) : qsTr(`add alt text to picture ${(index + 1)}`)
    }

    SvgButton {
        x: parent.width - width
        width: 34
        height: width
        svg: svgOutline.close
        accessibleName: qsTr("remove video")
        onClicked: videoAttachement.remove()
    }

    SkyLabel {
        y: parent.height - height
        width: parent.width
        backgroundColor: guiSettings.errorColor
        horizontalAlignment: Text.AlignHCenter
        color: "white"
        text: "ALT text missing"
        visible: requireAltText && !hasAltText()
    }

    SvgButton {
        x: parent.width - width
        y: parent.height - height
        width: 34
        height: 34
        svg: svgOutline.edit
        accessibleName: qsTr("edit video")
        onClicked: edit()
    }

    SkyLabel {
        x: 5
        y: parent.height - height - 5
        backgroundColor: "black"
        backgroundOpacity: 0.6
        color: "white"
        text: guiSettings.videoDurationToString(videoEndMs - videoStartMs)
    }

    GuiSettings {
        id: guiSettings
    }

    function remove() {
        postUtils.dropVideo(video)
        video = ""
        altText = ""
    }

    function hasAltText() {
        return Boolean(altText)
    }

    function editAltText() {
        let component = Qt.createComponent("AltTextEditor.qml")
        let altPage = component.createObject(page, {
            imgSource: video,
            sourceIsVideo: true,
            text: altText })
        altPage.onAltTextChanged.connect((text) => {
            altText = text
            root.popStack()
        })
        root.pushStack(altPage)
    }
}
