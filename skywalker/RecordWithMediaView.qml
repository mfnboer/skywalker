import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Item {
    required property record_with_media_view record
    required property int contentVisibility // QEnums::ContentVisibility
    required property string contentWarning
    property string backgroundColor: GuiSettings.backgroundColor

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
        }

        Component.onCompleted: {
            if (record.images.length > 0) {
                let qmlFile = `ImagePreview${(record.images.length)}.qml`
                mediaLoader.setSource(qmlFile, {
                                          images: record.images,
                                          contentVisibility: recordItem.contentVisibility,
                                          contentWarning: recordItem.contentWarning })
            }
            else if (record.video) {
                mediaLoader.setSource("VideoView.qml", {
                                          videoView: record.video,
                                          contentVisibility: recordItem.contentVisibility,
                                          contentWarning: recordItem.contentVisibility,
                                          backgroundColor: recordItem.backgroundColor })
            }
            else if (record.external) {
                mediaLoader.setSource("ExternalView.qml", {
                                            postExternal: record.external,
                                            contentVisibility: recordItem.contentVisibility,
                                            contentWarning: recordItem.contentVisibility })
            }
        }
    }


    function movedOffScreen() {
        if (record.video && mediaLoader.item)
            mediaLoader.item.pause()
    }
}
