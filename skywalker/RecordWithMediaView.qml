import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Item {
    required property record_with_media_view record
    required property int contentVisibility // QEnums::ContentVisibility
    required property string contentWarning
    property string backgroundColor: guiSettings.backgroundColor

    id: recordItem
    width: parent.width
    height: recordColumn.height + 10

    Column {
        id: recordColumn
        width: parent.width - 10
        anchors.centerIn: parent
        spacing: 5

        Loader {
            id: imageLoader
            width: parent.width
            visible: status == Loader.Ready
        }

        Loader {
            id: videoLoader
            width: parent.width
            visible: status == Loader.Ready
        }

        Loader {
            id: externalLoader
            width: parent.width
            visible: status == Loader.Ready
        }

        RecordView {
            record: recordItem.record.record
        }

        Component.onCompleted: {
            if (record.images.length > 0) {
                let qmlFile = `ImagePreview${(record.images.length)}.qml`
                imageLoader.setSource(qmlFile, {
                                          images: record.images,
                                          contentVisibility: recordItem.contentVisibility,
                                          contentWarning: recordItem.contentWarning })
            }

            if (record.video) {
                videoLoader.setSource("VideoView.qml", {
                                          videoView: record.video,
                                          contentVisibility: recordItem.contentVisibility,
                                          contentWarning: recordItem.contentVisibility,
                                          backgroundColor: recordItem.backgroundColor })
            }

            if (record.external) {
                externalLoader.setSource("ExternalView.qml", {
                                            postExternal: record.external,
                                            contentVisibility: recordItem.contentVisibility,
                                            contentWarning: recordItem.contentVisibility })
            }
        }
    }
    Rectangle {
        anchors.fill: parent
        border.width: 1
        border.color: guiSettings.borderColor
        color: "transparent"
        radius: 10
    }

    GuiSettings {
        id: guiSettings
    }

    function movedOffScreen() {
        if (videoLoader.item)
            videoLoader.item.pause()
    }
}
