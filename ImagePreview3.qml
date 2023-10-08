import QtQuick
import QtQuick.Layouts
import skywalker

RoundedFrame {
    required property int contentVisibility
    required property string contentWarning
    property list<imageview> images
    property int spacing: 4
    property bool showWarnedMedia: false
    property imageview nullImage

    id: frame
    objectToRound: imgGrid
    width: parent.width
    height: imageVisible() ? parent.width / 1.5 :
                contentVisibility === QEnums.CONTENT_VISIBILITY_WARN_MEDIA ? warnText.height : hideText.height

    Item {
        id: imgGrid
        anchors.fill: parent

        ThumbImageView {
            id: img1
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: parent.width / 1.5 - frame.spacing / 2
            height: parent.height
            Layout.fillWidth: true
            fillMode: Image.PreserveAspectCrop
            imageView: imageVisible() ? images[0] : nullImage
        }

        ThumbImageView {
            id: img2
            anchors.right: parent.right
            anchors.top: parent.top
            width: parent.width / 3 - frame.spacing / 2
            height: width
            Layout.fillWidth: true
            fillMode: Image.PreserveAspectCrop
            imageView: imageVisible() ? images[1] : nullImage
        }

        ThumbImageView {
            id: img3
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            width: parent.width / 3 - frame.spacing / 2
            height: width
            Layout.fillWidth: true
            fillMode: Image.PreserveAspectCrop
            imageView: imageVisible() ? images[2] : nullImage
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
