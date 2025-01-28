import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

Row {
    required property int contentVisibility // QEnums::ContentVisibility
    required property string contentWarning
    property list<imageview> images
    property bool showWarnedMedia: false
    property imageview nullImage
    property string imageUrl // Set instead of images list
    property bool isVideo: false
    readonly property string showMsg: isVideo ? qsTr("Show video") : qsTr("Show picture")

    height: !imageVisible() ? Math.max(imgIcon.height, contentVisibility === QEnums.CONTENT_VISIBILITY_WARN_MEDIA ? warnText.height : hideText.height) : 0
    spacing: 10

    Accessible.role: warnText.visible ? Accessible.Link : Accessible.StaticText
    Accessible.name: warnText.visible ? qsTr(`Hidden image content: ${contentWarning}. Press to show pictures`) : qsTr(`Hidden image content: ${contentWarning}`)
    Accessible.onPressAction: if (warnText.visible) warnText.linkActivated("")

    SkySvg {
        id: imgIcon
        width: 30
        height: width
        color: Material.color(Material.Grey)
        svg: SvgOutline.hideVisibility
        visible: !imageVisible()
    }

    Text {
        id: warnText
        width: parent.width
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        elide: Text.ElideRight
        textFormat: Text.RichText
        color: Material.color(Material.Grey)
        text: contentWarning + `<br><a href=\"show\" style=\"color: ${guiSettings.linkColor};\">` +
              (images.length === 1 || imageUrl ? showMsg : qsTr("Show pictures")) + "</a>"
        visible: contentVisibility === QEnums.CONTENT_VISIBILITY_WARN_MEDIA && !showWarnedMedia
        onLinkActivated: showWarnedMedia = true;
    }

    Text {
        id: hideText
        width: parent.width
        Layout.fillWidth: true
        anchors.verticalCenter: parent.verticalCenter
        wrapMode: Text.Wrap
        elide: Text.ElideRight
        textFormat: Text.RichText
        color: Material.color(Material.Grey)
        text: contentWarning
        visible: contentVisibility === QEnums.CONTENT_VISIBILITY_HIDE_MEDIA
    }

    function imageVisible() {
        return ![QEnums.CONTENT_VISIBILITY_HIDE_MEDIA,
                 QEnums.CONTENT_VISIBILITY_WARN_MEDIA].includes(contentVisibility) ||
               showWarnedMedia
    }

    function getImage(index) {
        return imageVisible() ? images[index] : nullImage
    }
}
