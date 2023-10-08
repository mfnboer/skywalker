import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

RoundedFrame {
    required property int contentVisibility
    required property string contentWarning
    property list<imageview> images
    property bool showWarnedMedia: false
    property imageview nullImage

    id: frame
    objectToRound: img
    width: parent.width
    height: imageVisible() ? img.height :
                contentVisibility === QEnums.CONTENT_VISIBILITY_WARN_MEDIA ? warnText.height : hideText.height

    ThumbImageView {
        id: img
        width: parent.width
        Layout.fillWidth: true
        fillMode: Image.PreserveAspectFit
        imageView: imageVisible() ? images[0] : nullImage

        onWidthChanged: setHeight()

        function setHeight() {
            let image = images[0]
            if (image.width > 0 && image.height > 0)
                heigth = image.height / image.width * width
        }
    }
    MouseArea {
        anchors.fill: img
        cursorShape: Qt.PointingHandCursor
        onClicked: root.viewFullImage(images, 0)
    }

    Text {
        id: warnText
        width: parent.width
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        elide: Text.ElideRight
        textFormat: Text.RichText
        color: "red"
        // TODO: icon
        text: qsTr("WARNING") + ": " + postContentWarning + "<br><a href=\"show\">" + qsTr("Show picture") + "</a>"
        visible: contentVisibility === QEnums.CONTENT_VISIBILITY_WARN_MEDIA && !showWarnedMedia
        onLinkActivated: showWarnedMedia = true
    }

    // TODO: we should show nothing at all when hidden.
    Text {
        id: hideText
        width: parent.width
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        elide: Text.ElideRight
        textFormat: Text.RichText
        color: "red"
        // TODO: icon
        text: qsTr("HIDDEN PICTURE") + ": " + contentWarning
        visible: contentVisibility === QEnums.CONTENT_VISIBILITY_HIDE_MEDIA
    }

    function imageVisible() {
        return ![QEnums.CONTENT_VISIBILITY_HIDE_MEDIA,
                 QEnums.CONTENT_VISIBILITY_WARN_MEDIA].includes(contentVisibility) ||
               showWarnedMedia
    }

    Component.onCompleted: {
        img.setHeight()
    }
}
