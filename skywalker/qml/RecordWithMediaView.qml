import QtQuick
import skywalker

Item {
    property string userDid
    required property record_with_media_view record
    property string backgroundColor: "transparent"
    required property int contentVisibility // QEnums::ContentVisibility
    required property string contentWarning
    required property basicprofile contentLabeler
    property bool highlight: false
    property bool isDraft: false
    property bool swipeMode: false
    property bool showRecord: true
    readonly property int margin: 10

    signal activateSwipe(int imgIndex, var previewImg)

    id: recordItem
    width: parent.width
    height: recordColumn.height

    Column {
        id: recordColumn
        width: parent.width
        anchors.centerIn: parent
        spacing: 5

        // See media loader in PostBody.qml
        Loader {
            id: mediaLoader
            x: swipeMode ? -margin - 1 : 0
            width: parent.width + (swipeMode ? 2 * margin + 2 : 0)
            visible: status == Loader.Ready
        }

        RecordView {
            userDid: recordItem.userDid
            record: recordItem.record.record
            backgroundColor: guiSettings.isLightMode ? Qt.darker(recordItem.backgroundColor, 1.05) : Qt.lighter(recordItem.backgroundColor, 1.5)
            visible: !recordItem.swipeMode && recordItem.showRecord
        }

        Component {
            id: images1Component

            ImagePreview1 {
                images: record.images
                maskColor: backgroundColor == "transparent" ? guiSettings.backgroundColor : backgroundColor
                contentVisibility: recordItem.contentVisibility
                contentWarning: recordItem.contentWarning
                contentLabeler: recordItem.contentLabeler
                swipeMode: recordItem.swipeMode

                onActivateSwipe: (imgIndex, previewImg) => recordItem.activateSwipe(imgIndex, previewImg)
            }
        }

        Component {
            id: images2Component

            ImagePreview2 {
                images: record.images
                maskColor: backgroundColor == "transparent" ? guiSettings.backgroundColor : backgroundColor
                contentVisibility: recordItem.contentVisibility
                contentWarning: recordItem.contentWarning
                contentLabeler: recordItem.contentLabeler
                swipeMode: recordItem.swipeMode

                onActivateSwipe: (imgIndex, previewImg) => recordItem.activateSwipe(imgIndex, previewImg)
            }
        }

        Component {
            id: images3Component

            ImagePreview3 {
                images: record.images
                maskColor: backgroundColor == "transparent" ? guiSettings.backgroundColor : backgroundColor
                contentVisibility: recordItem.contentVisibility
                contentWarning: recordItem.contentWarning
                contentLabeler: recordItem.contentLabeler
                swipeMode: recordItem.swipeMode

                onActivateSwipe: (imgIndex, previewImg) => recordItem.activateSwipe(imgIndex, previewImg)
            }
        }

        Component {
            id: images4Component

            ImagePreview4 {
                images: record.images
                maskColor: backgroundColor == "transparent" ? guiSettings.backgroundColor : backgroundColor
                contentVisibility: recordItem.contentVisibility
                contentWarning: recordItem.contentWarning
                contentLabeler: recordItem.contentLabeler
                swipeMode: recordItem.swipeMode

                onActivateSwipe: (imgIndex, previewImg) => recordItem.activateSwipe(imgIndex, previewImg)
            }
        }

        Component {
            id: videoThumbnailComponent

            VideoThumbnail {
                width: Math.min(180 * 1.777, record.width)
                height: 180
                videoSource: record.video.playlistUrl
            }
        }

        Component {
            id: videoViewComponent

            VideoView {
                videoView: record.video
                contentVisibility: recordItem.contentVisibility
                contentWarning: recordItem.contentWarning
                contentLabeler: recordItem.contentLabeler
                backgroundColor: recordItem.backgroundColor
                highlight: recordItem.highlight
                swipeMode: recordItem.swipeMode

                onActivateSwipe: (imgIndex, previewImg) => recordItem.activateSwipe(imgIndex, previewImg)
            }
        }

        Component {
            id: externalViewComponent

            ExternalView {
                userDid: recordItem.userDid
                postExternal: record.external
                contentVisibility: recordItem.contentVisibility
                contentWarning: recordItem.contentWarning
                contentLabeler: recordItem.contentLabeler
                highlight: recordItem.highlight
            }
        }

        Component {
            id: unknownEmbedComponent

            UnknownEmbedView {
                width: parent.width
                unknownEmbedType: record.unknownEmbedType
            }
        }

        Component.onCompleted: {
            if (record.images.length > 0) {
                const compList = [images1Component, images2Component, images3Component, images4Component]
                mediaLoader.sourceComponent = compList[record.images.length - 1]
                mediaLoader.active = true
            }
            else if (record.video) {
                if (isDraft) {
                    mediaLoader.sourceComponent = videoThumbnailComponent
                    mediaLoader.active = true
                }
                else {
                    mediaLoader.sourceComponent = videoViewComponent
                    mediaLoader.active = true
                }
            }
            else if (record.external) {
                mediaLoader.sourceComponent = externalViewComponent
                mediaLoader.active = true
            }
            else if (record.hasUnknownEmbed) {
                mediaLoader.sourceComponent = unknownEmbedComponent
                mediaLoader.active = true
            }
        }
    }


    function movedOffScreen() {
        if (record.video && mediaLoader.item)
            mediaLoader.item.pause()
    }
}
