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

    height: !imageVisible() ? Math.max(imgIconLoader.height, contentVisibility === QEnums.CONTENT_VISIBILITY_WARN_MEDIA ? warnColumnLoader.getHeight() : hideColumnLoader.getHeight()) : 0
    spacing: 10

    Loader {
        id: imgIconLoader
        width: 30
        height: width
        active: !imageVisible()

        sourceComponent: SkySvg {
            id: imgIcon
            color: Material.color(Material.Grey)
            svg: SvgOutline.hideVisibility
        }
    }

    Loader {
        id: warnColumnLoader
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width - imgIconLoader.width
        active: contentVisibility === QEnums.CONTENT_VISIBILITY_WARN_MEDIA && !showWarnedMedia

        sourceComponent: Column {
            id: warnColumn

            AccessibleText {
                id: warnText
                width: parent.width
                wrapMode: Text.Wrap
                textFormat: Text.RichText
                color: Material.color(Material.Grey)
                text: contentWarning + `<br><a href=\"show\" style=\"color: ${guiSettings.linkColor};\">` +
                      (images.length === 1 || imageUrl ? showMsg : qsTr("Show pictures")) + "</a>"
                onLinkActivated: showWarnedMedia = true;
            }
            AccessibleText {
                width: parent.width
                elide: Text.ElideRight
                color: Material.color(Material.Grey)
                font.pointSize: guiSettings.scaledFont(7/8)
                font.italic: true
                text: `@${contentLabeler.handle}`
                visible: !contentLabeler.isNull()
            }
        }

        function getHeight() {
            return item ? item.height : 0
        }
    }

    Loader {
        id: hideColumnLoader
        anchors.verticalCenter: parent.verticalCenter
        width: parent.width - imgIconLoader.width
        active: contentVisibility === QEnums.CONTENT_VISIBILITY_HIDE_MEDIA

        sourceComponent: Column {
            id: hideColumn

            AccessibleText {
                width: parent.width
                wrapMode: Text.Wrap
                textFormat: Text.RichText
                color: Material.color(Material.Grey)
                text: contentWarning
            }
            AccessibleText {
                width: parent.width
                elide: Text.ElideRight
                color: Material.color(Material.Grey)
                font.pointSize: guiSettings.scaledFont(7/8)
                font.italic: true
                text: `@${contentLabeler.handle}`
                visible: !contentLabeler.isNull()
            }
        }

        function getHeight() {
            return item ? item.height : 0
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
