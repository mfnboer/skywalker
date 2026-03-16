import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

Column {
    property string userDid
    readonly property int margin: 10
    required property basicprofile postAuthor
    required property bool postHasUnknownEmbed
    required property string postUnknownEmbedType
    required property list<imageview> postImages
    required property list<contentlabel> postContentLabels
    property contentlabel filteredContentLabel
    required property int postContentVisibility // QEnums::PostContentVisibility
    required property string postContentWarning
    required property basicprofile postContentLabeler
    required property int postMuted // QEnums::MutedPostReason
    property var postVideo // videoView
    property var postExternal // externalview (var allows NULL)
    property var postRecord // recordview
    property var postRecordWithMedia // record_with_media_view
    property string bodyBackgroundColor: guiSettings.backgroundColor
    property bool showWarnedPost: false
    property bool mutePost: postMuted !== QEnums.MUTED_POST_NONE
    property string postHighlightColor: "transparent"
    property bool isDraft: false
    property bool swipeMode: false
    property bool showRecord: true
    property bool postVisible: true

    signal activateSwipe(int imgIndex, var previewImg)

    id: postBody

    // Images
    LoaderImagePreview1 {
        id: images1Loader
        x: swipeMode ? -margin : 0
        width: parent.width + (swipeMode ? 2 * margin : 0)
        postImages: postBody.postImages
        postContentVisibility: postBody.postContentVisibility
        postContentWarning: postBody.postContentWarning
        postContentLabeler: postBody.postContentLabeler
        bodyBackgroundColor: postBody.bodyBackgroundColor
        swipeMode: postBody.swipeMode
        postVisible: postBody.postVisible

        onActivateSwipe: (imgIndex, previewImg) => postBody.activateSwipe(imgIndex, previewImg)
    }
    LoaderImagePreview2 {
        id: images2Loader
        x: swipeMode ? -margin : 0
        width: parent.width + (swipeMode ? 2 * margin : 0)
        postImages: postBody.postImages
        postContentVisibility: postBody.postContentVisibility
        postContentWarning: postBody.postContentWarning
        postContentLabeler: postBody.postContentLabeler
        bodyBackgroundColor: postBody.bodyBackgroundColor
        swipeMode: postBody.swipeMode
        postVisible: postBody.postVisible

        onActivateSwipe: (imgIndex, previewImg) => postBody.activateSwipe(imgIndex, previewImg)
    }
    LoaderImagePreview3 {
        id: images3Loader
        x: swipeMode ? -margin : 0
        width: parent.width + (swipeMode ? 2 * margin : 0)
        postImages: postBody.postImages
        postContentVisibility: postBody.postContentVisibility
        postContentWarning: postBody.postContentWarning
        postContentLabeler: postBody.postContentLabeler
        bodyBackgroundColor: postBody.bodyBackgroundColor
        swipeMode: postBody.swipeMode
        postVisible: postBody.postVisible

        onActivateSwipe: (imgIndex, previewImg) => postBody.activateSwipe(imgIndex, previewImg)
    }
    LoaderImagePreview4 {
        id: images4Loader
        x: swipeMode ? -margin : 0
        width: parent.width + (swipeMode ? 2 * margin : 0)
        postImages: postBody.postImages
        postContentVisibility: postBody.postContentVisibility
        postContentWarning: postBody.postContentWarning
        postContentLabeler: postBody.postContentLabeler
        bodyBackgroundColor: postBody.bodyBackgroundColor
        swipeMode: postBody.swipeMode
        postVisible: postBody.postVisible

        onActivateSwipe: (imgIndex, previewImg) => postBody.activateSwipe(imgIndex, previewImg)
    }

    // Video
    // HACK: somehow video leaves 1 empty pixel at each side. Add 2 pixels to fix it.
    LoaderVideoPreview {
        id: videoLoader
        postVideo: postBody.postVideo
        postContentVisibility: postBody.postContentVisibility
        postContentWarning: postBody.postContentWarning
        postContentLabeler: postBody.postContentLabeler
        bodyBackgroundColor: postBody.bodyBackgroundColor
        swipeMode: postBody.swipeMode
        isDraft: postBody.isDraft
        postVisible: postBody.postVisible

        onActivateSwipe: (imgIndex, previewImg) => postBody.activateSwipe(imgIndex, previewImg)
    }

    // External
    LoaderExternal {
        width: parent.width
        userDid: postBody.userDid
        postExternal: postBody.postExternal
        postContentVisibility: postBody.postContentVisibility
        postContentWarning: postBody.postContentWarning
        postContentLabeler: postBody.postContentLabeler
        bodyBackgroundColor: postBody.bodyBackgroundColor
        postVisible: postBody.postVisible
    }

    // Unknown embed
    LoaderUnknownEmbed {
        width: parent.width
        postHasUnknownEmbed: postBody.postHasUnknownEmbed
        postUnknownEmbedType: postBody.postUnknownEmbedType
        postVisible: postBody.postVisible
    }

    LoaderContentLabels {
        width: parent.width
        postAuthor: postBody.postAuthor
        postContentLabels: postBody.postContentLabels
        filteredContentLabel: postBody.filteredContentLabel
        postVisible: postBody.postVisible
    }

    // Record
    Loader {
        id: recordLoader
        width: parent.width
        active: Boolean(postRecord) && showRecord && postVisible

        sourceComponent: RecordView {
            userDid: postBody.userDid
            record: postRecord
            backgroundColor: guiSettings.highLightColor(bodyBackgroundColor)
        }
    }

    // Record with media
    Loader {
        id: recordWithMediaLoader
        width: parent.width
        active: Boolean(postRecordWithMedia) && postVisible

        sourceComponent: RecordWithMediaView {
            userDid: postBody.userDid
            record: postRecordWithMedia
            backgroundColor: guiSettings.highLightColor(bodyBackgroundColor)
            contentVisibility: postContentVisibility
            contentWarning: postContentWarning
            contentLabeler: postContentLabeler
            highlight: bodyBackgroundColor === guiSettings.postHighLightColor
            isDraft: postBody.isDraft
            swipeMode: postBody.swipeMode
            showRecord: postBody.showRecord

            onActivateSwipe: (imgIndex, img) => postBody.activateSwipe(imgIndex, img)
        }
    }

    function closeMedia(mediaIndex, closeCb) {
        if (images1Loader.item)
            images1Loader.item.closeMedia(mediaIndex, closeCb)
        else if (images2Loader.item)
            images2Loader.item.closeMedia(mediaIndex, closeCb)
        else if (images3Loader.item)
            images3Loader.item.closeMedia(mediaIndex, closeCb)
        else if (images4Loader.item)
            images4Loader.item.closeMedia(mediaIndex, closeCb)
        else if (videoLoader.item)
            videoLoader.item.closeMedia(mediaIndex, closeCb)
        else
            closeCb()
    }

    function movedOffScreen() {
        if (videoLoader.item)
            videoLoader.item.pause() // qmllint disable missing-property

        if (recordLoader.item)
            recordLoader.item.movedOffScreen()

        if (recordWithMediaLoader.item)
            recordWithMediaLoader.item.movedOffScreen()
    }

    onBodyBackgroundColorChanged: {
        if (recordLoader.item) {
            recordLoader.item.backgroundColor = bodyBackgroundColor
            recordLoader.item.highlight = bodyBackgroundColor === guiSettings.postHighLightColor
        }

        if (recordWithMediaLoader.item) {
            recordWithMediaLoader.item.backgroundColor = bodyBackgroundColor
            recordWithMediaLoader.item.highlight = bodyBackgroundColor === guiSettings.postHighLightColor
        }
    }
}
