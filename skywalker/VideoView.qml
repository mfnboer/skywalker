import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia
import skywalker

Rectangle {
    required property var videoView // videoView
    required property int contentVisibility // QEnums::ContentVisibility
    required property string contentWarning

    id: videoRect
    width: parent.width
    height: videoPreview.height

    RoundedFrame {
        id: videoPreview
        width: parent.width
        objectToRound: videoColumn
        border.width: 1
        border.color: guiSettings.borderColor
        visible: !videoLoader.active

        FilteredImageWarning {
            id: filter
            width: parent.width - 2
            contentVisibiliy: videoRect.contentVisibility
            contentWarning: videoRect.contentWarning
            imageUrl: videoView.thumbUrl
        }

        Column {
            id: videoColumn
            width: parent.width
            topPadding: 1
            spacing: 3

            // HACK: The filter should be in this place, but inside a rounded object links
            // cannot be clicked.
            Rectangle {
                width: filter.width
                height: filter.height
                color: "transparent"
            }
            ImageAutoRetry {
                id: thumbImg
                x: 1
                width: parent.width - 2
                source: filter.imageVisible() ? videoView.thumbUrl : ""
                fillMode: Image.PreserveAspectFit
            }
        }

        SvgButton {
            x: (parent.width - width) / 2
            y: (parent.height - height) / 2
            width: 70
            height: width
            imageMargin: 0
            opacity: 0.5
            accessibleName: qsTr("play video")
            svg: svgFilled.play

            onClicked: videoLoader.active = true
        }
    }

    Loader {
        id: videoLoader
        width: parent.width
        height: parent.height
        active: false
        visible: status == Loader.Ready
        sourceComponent: Video {
            id: video
            width: parent.width
            height: parent.height
            source: videoView.playlistUrl
            fillMode: VideoOutput.PreserveAspectFit

            onPlaying: console.debug("PLAYING")
        }

        onStatusChanged: {
            if (status == Loader.Ready) {
                console.debug("START")
                item.play()
            }
            else
            {
                console.debug("STATUS:", status)
            }
        }
    }

    GuiSettings {
        id: guiSettings
    }
}
