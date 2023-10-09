import QtQuick
import QtQuick.Layouts
import skywalker

Row {
    required property int contentVisibiliy // QEnums::ContentVisibility
    required property string contentWarning
    property list<imageview> images
    property bool showWarnedMedia: false
    property imageview nullImage

    height: Math.max(imgIcon.height, contentVisibility === QEnums.CONTENT_VISIBILITY_WARN_MEDIA ? warnText.height : hideText.height)
    spacing: 10

    SvgImage {
        id: imgIcon
        width: 30
        height: width
        color: "grey"
        svg: svgOutline.hideVisibility
        visible: !imageVisible()
    }

    Text {
        id: warnText
        width: parent.width
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        elide: Text.ElideRight
        textFormat: Text.RichText
        color: "grey"
        text: contentWarning + "<br><a href=\"show\">" +
              (images.length === 1 ? qsTr("Show picture") : qsTr("Show pictures")) + "</a>"
        visible: contentVisibility === QEnums.CONTENT_VISIBILITY_WARN_MEDIA && !showWarnedMedia
        onLinkActivated: showWarnedMedia = true
    }

    Text {
        id: hideText
        width: parent.width
        Layout.fillWidth: true
        anchors.verticalCenter: parent.verticalCenter
        wrapMode: Text.Wrap
        elide: Text.ElideRight
        textFormat: Text.RichText
        color: "grey"
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
