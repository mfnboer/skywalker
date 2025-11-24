import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

Row {
    required property int contentVisibility // QEnums::ContentVisibility
    required property string contentWarning
    required property basicprofile contentLabeler
    property list<imageview> images
    property bool showWarnedMedia: false
    property imageview nullImage
    property string imageUrl // Set instead of images list
    property bool isVideo: false
    readonly property string showMsg: isVideo ? qsTr("Show video") : qsTr("Show picture")

    height: !imageVisible() ? Math.max(imgIcon.height, contentVisibility === QEnums.CONTENT_VISIBILITY_WARN_MEDIA ? warnColumn.height : hideColumn.height) : 0
    spacing: 10

    Accessible.role: warnColumn.visible ? Accessible.Link : Accessible.StaticText
    Accessible.name: warnColumn.visible ? qsTr(`Hidden image content: ${contentWarning}. Press to show pictures`) : qsTr(`Hidden image content: ${contentWarning}`)
    Accessible.onPressAction: if (warnColumn.visible) warnText.linkActivated("")

    SkySvg {
        id: imgIcon
        width: 30
        height: width
        color: Material.color(Material.Grey)
        svg: SvgOutline.hideVisibility
        visible: !imageVisible()
    }

    Column {
        id: warnColumn
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width - imgIcon.width
        visible: contentVisibility === QEnums.CONTENT_VISIBILITY_WARN_MEDIA && !showWarnedMedia

        Text {
            id: warnText
            width: parent.width
            wrapMode: Text.Wrap
            textFormat: Text.RichText
            color: Material.color(Material.Grey)
            text: contentWarning + `<br><a href=\"show\" style=\"color: ${guiSettings.linkColor};\">` +
                  (images.length === 1 || imageUrl ? showMsg : qsTr("Show pictures")) + "</a>"
            onLinkActivated: showWarnedMedia = true;
        }
        Text {
            width: parent.width
            elide: Text.ElideRight
            color: Material.color(Material.Grey)
            font.pointSize: guiSettings.scaledFont(7/8)
            font.italic: true
            text: `@${contentLabeler.handle}`
            visible: !contentLabeler.isNull()
        }
    }

    Column {
        id: hideColumn
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width - imgIcon.width
        visible: contentVisibility === QEnums.CONTENT_VISIBILITY_HIDE_MEDIA

        Text {
            width: parent.width
            wrapMode: Text.Wrap
            textFormat: Text.RichText
            color: Material.color(Material.Grey)
            text: contentWarning
        }
        Text {
            width: parent.width
            elide: Text.ElideRight
            color: Material.color(Material.Grey)
            font.pointSize: guiSettings.scaledFont(7/8)
            font.italic: true
            text: `@${contentLabeler.handle}`
            visible: !contentLabeler.isNull()
        }
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
