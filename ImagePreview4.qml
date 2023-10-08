import QtQuick
import QtQuick.Layouts
import skywalker

RoundedFrame {
    required property int contentVisibility
    required property string contentWarning
    property list<imageview> images
    property bool showWarnedMedia: false
    property imageview nullImage

    objectToRound: imgGrid
    width: parent.width
    height: imageVisible() ? width :
                contentVisibility === QEnums.CONTENT_VISIBILITY_WARN_MEDIA ? warnText.height : hideText.height

    Grid {
        id: imgGrid
        anchors.fill: parent
        columns: 2
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

        ThumbImageView {
            id: img3
            width: parent.width / 2 - parent.spacing / 2
            height: width
            Layout.fillWidth: true
            fillMode: Image.PreserveAspectCrop
            imageView: imageVisible() ? images[2] : nullImage
        }

        ThumbImageView {
            id: img4
            width: parent.width / 2 - parent.spacing / 2
            height: width
            Layout.fillWidth: true
            fillMode: Image.PreserveAspectCrop
            imageView: imageVisible() ? images[3] : nullImage
        }
    }
    MouseArea {
        anchors.fill: imgGrid
        cursorShape: Qt.PointingHandCursor
        onClicked: {
            let p = Qt.point(mouseX, mouseY)
            let index = -1

            if (img1.contains(mapToItem(img1, p)))
                index = 0
            else if (img2.contains(mapToItem(img2, p)))
                index = 1
            else if (img3.contains(mapToItem(img3, p)))
                index = 2
            else if (img4.contains(mapToItem(img4, p)))
                index = 3

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

