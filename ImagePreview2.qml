import QtQuick
import QtQuick.Layouts
import skywalker

RoundedFrame {
    required property int contentVisibility
    required property string contentWarning
    property list<imageview> images
    property bool showWarnedMedia: false
    property imageview nullImage


    objectToRound: imgRow
    width: parent.width
    height: imageVisible() ? width / 2 :
                contentVisibility === QEnums.CONTENT_VISIBILITY_WARN_MEDIA ? warnText.height : hideText.height

    Row {
        id: imgRow
        anchors.fill: parent
        spacing: 4

        ThumbImageView {
            id: img1
            width: parent.width / 2 - parent.spacing / 2
            height: width
            Layout.fillWidth: true
            fillMode: Image.PreserveAspectCrop
            imageView: imageVisible() ? images[0] : nullImage
        }

        ThumbImageView {
            id: img2
            width: parent.width / 2 - parent.spacing / 2
            height: width
            Layout.fillWidth: true
            fillMode: Image.PreserveAspectCrop
            imageView: imageVisible() ? images[1] : nullImage
        }
    }
    MouseArea {
        anchors.fill: imgRow
        cursorShape: Qt.PointingHandCursor
        onClicked: {
            let p = Qt.point(mouseX, mouseY)
            let index = -1

            if (img1.contains(mapToItem(img1, p)))
                index = 0
            else if (img2.contains(mapToItem(img2, p)))
                index = 1

            if (index >= 0)
                root.viewFullImage(images, index)
        }
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
        text: qsTr("WARNING") + ": " + postContentWarning + "<br><a href=\"show\">" + qsTr("Show pictures") + "</a>"
        visible: contentVisibility === QEnums.CONTENT_VISIBILITY_WARN_MEDIA && !showWarnedMedia
        onLinkActivated: showWarnedMedia = true
    }

    Text {
        id: hideText
        width: parent.width
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        elide: Text.ElideRight
        textFormat: Text.RichText
        color: "red"
        // TODO: icon
        text: qsTr("HIDDEN PICTURES") + ": " + contentWarning
        visible: contentVisibility === QEnums.CONTENT_VISIBILITY_HIDE_MEDIA
    }

    function imageVisible() {
        return ![QEnums.CONTENT_VISIBILITY_HIDE_MEDIA,
                 QEnums.CONTENT_VISIBILITY_WARN_MEDIA].includes(contentVisibility) ||
               showWarnedMedia
    }
}
