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

        // Images
        LoaderImagePreview1 {
            id: images1Loader
            x: swipeMode ? -margin : 0
            width: parent.width + (swipeMode ? 2 * margin : 0)
            postImages: record.images
            postContentVisibility: contentVisibility
            postContentWarning: contentWarning
            postContentLabeler: contentLabeler
            bodyBackgroundColor: backgroundColor == "transparent" ? guiSettings.backgroundColor : backgroundColor
            swipeMode: recordItem.swipeMode

            onActivateSwipe: (imgIndex, previewImg) => recordItem.activateSwipe(imgIndex, previewImg)
        }
        LoaderImagePreview2 {
            id: images2Loader
            x: swipeMode ? -margin : 0
            width: parent.width + (swipeMode ? 2 * margin : 0)
            postImages: record.images
            postContentVisibility: contentVisibility
            postContentWarning: contentWarning
            postContentLabeler: contentLabeler
            bodyBackgroundColor: backgroundColor == "transparent" ? guiSettings.backgroundColor : backgroundColor
            swipeMode: recordItem.swipeMode

            onActivateSwipe: (imgIndex, previewImg) => recordItem.activateSwipe(imgIndex, previewImg)
        }
        LoaderImagePreview3 {
            id: images3Loader
            x: swipeMode ? -margin : 0
            width: parent.width + (swipeMode ? 2 * margin : 0)
            postImages: record.images
            postContentVisibility: contentVisibility
            postContentWarning: contentWarning
            postContentLabeler: contentLabeler
            bodyBackgroundColor: backgroundColor == "transparent" ? guiSettings.backgroundColor : backgroundColor
            swipeMode: recordItem.swipeMode

            onActivateSwipe: (imgIndex, previewImg) => recordItem.activateSwipe(imgIndex, previewImg)
        }
        LoaderImagePreview4 {
            id: images4Loader
            x: swipeMode ? -margin : 0
            width: parent.width + (swipeMode ? 2 * margin : 0)
            postImages: record.images
            postContentVisibility: contentVisibility
            postContentWarning: contentWarning
            postContentLabeler: contentLabeler
            bodyBackgroundColor: backgroundColor == "transparent" ? guiSettings.backgroundColor : backgroundColor
            swipeMode: recordItem.swipeMode

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
        }

        LoaderUnknownEmbed {
            width: parent.width
            postHasUnknownEmbed: record.hasUnknownEmbed
            postUnknownEmbedType: record.unknownEmbedType
        }

        RecordView {
            userDid: recordItem.userDid
            record: recordItem.record.record
            backgroundColor: guiSettings.highLightColor(recordItem.backgroundColor)
            visible: !recordItem.swipeMode && recordItem.showRecord
        }
    }

    function movedOffScreen() {
        if (record.video && videoLoader.item)
            videoLoader.item.pause()
    }
}
