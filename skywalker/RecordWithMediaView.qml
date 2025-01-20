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

        Component.onCompleted: {
            if (record.images.length > 0) {
                let qmlFile = `ImagePreview${(record.images.length)}.qml`
                mediaLoader.setSource(qmlFile, {
                                          images: record.images,
                                          maskColor: backgroundColor == "transparent" ? guiSettings.backgroundColor : backgroundColor,
                                          contentVisibility: recordItem.contentVisibility,
                                          contentWarning: recordItem.contentWarning })
            }
            else if (record.video) {
                mediaLoader.setSource("VideoView.qml", {
                                          videoView: record.video,
                                          contentVisibility: recordItem.contentVisibility,
                                          contentWarning: recordItem.contentVisibility,
                                          backgroundColor: backgroundColor,
                                          highlight: recordItem.highlight,
                                          isVideoFeed: recordItem.isVideoFeed })
            }
            else if (record.external) {
                mediaLoader.setSource("ExternalView.qml", {
                                          postExternal: record.external,
                                          contentVisibility: recordItem.contentVisibility,
                                          contentWarning: recordItem.contentVisibility,
                                          highlight: recordItem.highlight })
            }
        }
    }


    function movedOffScreen() {
        if (record.video && mediaLoader.item)
            mediaLoader.item.pause()
    }
}
