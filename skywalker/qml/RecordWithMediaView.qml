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
    property bool moving: false

    signal activateSwipe(int imgIndex, var previewImg)

    id: recordItem
    width: parent.width
    height: recordColumn.height

    Column {
        id: recordColumn
        width: parent.width
        anchors.centerIn: parent
        spacing: 5

        // Images
        LoaderImageGridPreview {
            id: imagesLoader
            x: swipeMode ? -margin : 0
            width: parent.width + (swipeMode ? 2 * margin : 0)
            postImages: record.images
            postContentVisibility: contentVisibility
            postContentWarning: contentWarning
            postContentLabeler: contentLabeler
            bodyBackgroundColor: backgroundColor == "transparent" ? guiSettings.backgroundColor : backgroundColor
            swipeMode: recordItem.swipeMode
            moving: recordItem.moving

            onActivateSwipe: (imgIndex, previewImg) => recordItem.activateSwipe(imgIndex, previewImg)
        }

        LoaderVideoPreview {
            id: videoLoader
            postVideo: record.video
            postContentVisibility: contentVisibility
            postContentWarning: contentWarning
            postContentLabeler: contentLabeler
            bodyBackgroundColor: recordItem.backgroundColor
            swipeMode: recordItem.swipeMode
            isDraft: recordItem.isDraft
            moving: recordItem.moving

            onActivateSwipe: (imgIndex, previewImg) => recordItem.activateSwipe(imgIndex, previewImg)
        }

        LoaderExternal {
            width: parent.width
            userDid: recordItem.userDid
            postExternal: record.external
            postContentVisibility: contentVisibility
            postContentWarning: contentWarning
            postContentLabeler: contentLabeler
            highlight: recordItem.highlight
            moving: recordItem.moving
        }

        LoaderUnknownEmbed {
            width: parent.width
            postHasUnknownEmbed: record.hasUnknownEmbed
            postUnknownEmbedType: record.unknownEmbedType
        }

        Loader {
            width: parent.width
            active: !recordItem.swipeMode && recordItem.showRecord

            sourceComponent: RecordView {
                userDid: recordItem.userDid
                record: recordItem.record.record
                backgroundColor: guiSettings.highLightColor(recordItem.backgroundColor)
                moving: recordItem.moving
            }
        }
    }

    function movedOffScreen() {
        if (record.video && videoLoader.item)
            videoLoader.item.pause()
    }
}
