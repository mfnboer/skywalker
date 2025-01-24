import QtQuick
import skywalker

Item {
    required property record_with_media_view record
    property string backgroundColor: "transparent"
    required property int contentVisibility // QEnums::ContentVisibility
    required property string contentWarning
    property bool highlight: false
    property bool isVideoFeed: false

    id: recordItem
    width: parent.width
    height: recordColumn.height

    Column {
        id: recordColumn
        width: parent.width
        anchors.centerIn: parent
        spacing: 5

        Loader {
            id: mediaLoader
            width: parent.width
            visible: status == Loader.Ready
        }

        RecordView {
            record: recordItem.record.record
            backgroundColor: recordItem.backgroundColor
            highlight: recordItem.highlight
        }

        Component {
            id: images1Component

            ImagePreview1 {
                images: record.images
                maskColor: backgroundColor == "transparent" ? guiSettings.backgroundColor : backgroundColor
                contentVisibility: recordItem.contentVisibility
                contentWarning: recordItem.contentWarning
            }
        }

        Component {
            id: images2Component

            ImagePreview2 {
                images: record.images
                maskColor: backgroundColor == "transparent" ? guiSettings.backgroundColor : backgroundColor
                contentVisibility: recordItem.contentVisibility
                contentWarning: recordItem.contentWarning
            }
        }

        Component {
            id: images3Component

            ImagePreview3 {
                images: record.images
                maskColor: backgroundColor == "transparent" ? guiSettings.backgroundColor : backgroundColor
                contentVisibility: recordItem.contentVisibility
                contentWarning: recordItem.contentWarning
            }
        }

        Component {
            id: images4Component

            ImagePreview4 {
                images: record.images
                maskColor: backgroundColor == "transparent" ? guiSettings.backgroundColor : backgroundColor
                contentVisibility: recordItem.contentVisibility
                contentWarning: recordItem.contentWarning
            }
        }

        Component {
            id: videoViewComponent

            VideoView {
                videoView: record.video
                contentVisibility: recordItem.contentVisibility
                contentWarning: recordItem.contentVisibility
                backgroundColor: backgroundColor
                highlight: recordItem.highlight
                isVideoFeed: recordItem.isVideoFeed
            }
        }

        Component {
            id: externalViewComponent

            ExternalView {
                postExternal: record.external
                contentVisibility: recordItem.contentVisibility
                contentWarning: recordItem.contentVisibility
                highlight: recordItem.highlight
            }
        }

        Component.onCompleted: {
            if (record.images.length > 0) {
                const compList = [images1Component, images2Component, images3Component, images4Component]
                mediaLoader.sourceComponent = compList[record.images.length - 1]
                mediaLoader.active = true
            }
            else if (record.video) {
                mediaLoader.sourceComponent = videoViewComponent
                mediaLoader.active = true
            }
            else if (record.external) {
                mediaLoader.sourceComponent = externalViewComponent
                mediaLoader.active = true
            }
        }
    }


    function movedOffScreen() {
        if (record.video && mediaLoader.item)
            mediaLoader.item.pause()
    }
}
