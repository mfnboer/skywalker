import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

Column {
    property string userDid
    readonly property int margin: 10
    required property basicprofile postAuthor
    required property string postText
    required property bool postHasUnknownEmbed
    required property string postUnknownEmbedType
    required property list<imageview> postImages
    required property date postDateTime
    required property list<language> postLanguageLabels
    required property list<contentlabel> postContentLabels
    property contentlabel filteredContentLabel
    required property int postContentVisibility // QEnums::PostContentVisibility
    required property string postContentWarning
    required property basicprofile postContentLabeler
    required property int postMuted // QEnums::MutedPostReason
    required property string postPlainText
    required property bool postIsThread
    required property bool postIsThreadReply
    property var postVideo // videoView
    property var postExternal // externalview (var allows NULL)
    property var postRecord // recordview
    property var postRecordWithMedia // record_with_media_view
    property bool detailedView: false
    property int initialShowMaxTextLines: 25
    property int maxTextLines: 10000
    property string bodyBackgroundColor: guiSettings.backgroundColor
    property string borderColor: guiSettings.borderColor
    property bool showWarnedPost: false
    property bool mutePost: postMuted !== QEnums.MUTED_POST_NONE
    property bool attachmentsInitialized: false
    property string postHighlightColor: "transparent"
    property bool isDraft: false
    property bool swipeMode: false
    property bool showRecord: true
    readonly property bool showThreadIndicator: postIsThread && !postPlainText.includes(UnicodeFonts.THREAD_SYMBOL)
    readonly property bool replaceThreadIndicator: (postIsThread || postIsThreadReply) && !showThreadIndicator

    // The font-size is set to make sure the thread indicator is in normal text size when the
    // post is giant emoji only.
    // The <div> cause a line break if there is post text before. In an empty post no newline
    // is prepended.
    readonly property string displayText:
        (replaceThreadIndicator ? UnicodeFonts.turnLastThreadSymbolIntoLink(postText) : postText) +
        (showThreadIndicator ? `<a href="${UnicodeFonts.THREAD_LINK}" style="text-decoration: none; font-size: ${Application.font.pixelSize}px">${UnicodeFonts.THREAD_SYMBOL}</a>` : "")

    signal activateSwipe(int imgIndex, var previewImg)
    signal unrollThread

    id: postBody

    SkyCleanedText {
        id: bodyText
        width: parent.width
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        initialShowMaxLineCount: Math.min(maxTextLines, initialShowMaxTextLines)
        maximumLineCount: maxTextLines
        ellipsisBackgroundColor: postBody.bodyBackgroundColor
        elide: Text.ElideRight
        textFormat: Text.RichText
        color: guiSettings.textColor
        font.pointSize: getPostFontSize()
        plainText: displayText
        bottomPadding: postImages.length > 0 || postVideo || postExternal || postRecord || postRecordWithMedia || postHasUnknownEmbed ? 5 : 0
        visible: postVisible() && displayText

        Accessible.ignored: true

        LinkCatcher {
            z: parent.z - 1
            containingText: postPlainText
            userDid: postBody.userDid

            onUnrollThread: postBody.unrollThread()
        }

        Rectangle {
            anchors.fill: parent
            z: parent.z - 2
            radius: 5
            color: postHighlightColor
            opacity: guiSettings.focusHighlightOpacity
        }
    }

    Loader {
        width: parent.width
        active: !postVisible()
        visible: status == Loader.Ready
        sourceComponent: Row {
            width: parent.width
            spacing: 10

            SkySvg {
                id: imgIcon
                width: 30
                height: width
                color: Material.color(Material.Grey)
                svg: getIcon()

                function getIcon() {
                    if (!mutePost)
                        return SvgOutline.hideVisibility

                    switch (postMuted) {
                    case QEnums.MUTED_POST_AUTHOR:
                        return SvgOutline.mute
                    case QEnums.MUTED_POST_WORDS:
                        return SvgOutline.mutedWords
                    }

                    return SvgOutline.hideVisibility
                }
            }

            // The content warning is shown when the post is not muted
            Column {
                width: parent.width
                anchors.verticalCenter: parent.verticalCenter
                visible: postContentVisibility === QEnums.CONTENT_VISIBILITY_WARN_POST && !showWarnedPost && !mutePost

                Text {
                    width: parent.width
                    wrapMode: Text.Wrap
                    elide: Text.ElideRight
                    textFormat: Text.RichText
                    color: Material.color(Material.Grey)
                    text: postContentWarning
                }
                Text {
                    width: parent.width
                    elide: Text.ElideRight
                    textFormat: Text.RichText
                    font.pointSize: guiSettings.scaledFont(7/8)
                    font.italic: true
                    text: qsTr(`<a href="link" style="color: ${guiSettings.linkColor}; text-decoration: none">@${postContentLabeler.handle}</a>`)
                    visible: !postContentLabeler.isNull()
                    onLinkActivated: root.getSkywalker().getDetailedProfile(postContentLabeler.did)
                }
                Text {
                    topPadding: 20
                    width: parent.width
                    elide: Text.ElideRight
                    textFormat: Text.RichText
                    text: `<a href=\"show\" style=\"color: ${guiSettings.linkColor};\">` + qsTr("Show post") + "</a>"
                    onLinkActivated: {
                        showWarnedPost = true

                        if (postVisible())
                            showPostAttachements()
                    }
                }
            }

            // If the post is muted, then this takes precendence over the content warning
            Column {
                width: parent.width
                anchors.verticalCenter: parent.verticalCenter
                visible: mutePost && postContentVisibility !== QEnums.CONTENT_VISIBILITY_HIDE_POST

                Text {
                    width: parent.width
                    wrapMode: Text.Wrap
                    elide: Text.ElideRight
                    textFormat: Text.RichText
                    color: Material.color(Material.Grey)
                    text: getMuteText()
                }
                Text {
                    topPadding: 20
                    width: parent.width
                    elide: Text.ElideRight
                    textFormat: Text.RichText
                    text: `<a href=\"show\" style=\"color: ${guiSettings.linkColor};\">` + qsTr("Show post") + "</a>"
                    onLinkActivated: {
                        mutePost = false

                        // The post may still not be visible due to content filtering
                        if (postVisible())
                            showPostAttachements()
                    }
                }
            }

            // If a post is hidden then this text will show no matter whether the post is muted
            Column {
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width
                visible: postContentVisibility === QEnums.CONTENT_VISIBILITY_HIDE_POST

                Text {
                    width: parent.width
                    wrapMode: Text.Wrap
                    elide: Text.ElideRight
                    textFormat: Text.RichText
                    color: Material.color(Material.Grey)
                    text: postContentWarning
                }
                Text {
                    width: parent.width
                    textFormat: Text.RichText
                    elide: Text.ElideRight
                    color: Material.color(Material.Grey)
                    font.pointSize: guiSettings.scaledFont(7/8)
                    font.italic: true
                    text: qsTr(`<a href="link" style="color: ${guiSettings.linkColor}; text-decoration: none">@${postContentLabeler.handle}</a>`)
                    visible: !postContentLabeler.isNull()
                    onLinkActivated: root.getSkywalker().getDetailedProfile(postContentLabeler.did)
                }
            }
        }
    }

    Loader {
        id: languageLabelsLoader
        anchors.right: parent.right
        visible: status == Loader.Ready
    }

    // HACK: somehow video leaves 1 empty pixel at each side. Add 2 pixels to fix it.
    Loader {
        id: mediaLoader
        x: swipeMode ? -margin - 1 : 0
        width: parent.width + (swipeMode ? 2 * margin + 2 : 0)
        visible: status == Loader.Ready
    }

    Loader {
        id: contentLabelsLoader
        anchors.right: parent.right
        visible: status == Loader.Ready
    }

    Loader {
        id: recordLoader
        width: parent.width

        onItemChanged: {
            if (item && typeof item.onActivateSwipe != 'undefined')
                item.onActivateSwipe.connect(postBody.activateSwipe)
        }
    }

    Loader {
        id: dateTimeLoader
        width: parent.width
        active: detailedView
        visible: status == Loader.Ready
        sourceComponent: Text {
            width: parent.width
            topPadding: 10
            Layout.fillWidth: true
            elide: Text.ElideRight
            color: Material.color(Material.Grey)
            text: postDateTime.toLocaleString(Qt.locale(), Locale.ShortFormat)
            font.pointSize: guiSettings.scaledFont(7/8)
        }
    }

    function movedOffScreen() {
        if (postVideo && mediaLoader.item)
            mediaLoader.item.pause() // qmllint disable missing-property

        if ((postRecord || postRecordWithMedia) && recordLoader.item)
            recordLoader.item.movedOffScreen()
    }

    function postVisible() {
        if (mutePost)
            return false

        return ![QEnums.CONTENT_VISIBILITY_HIDE_POST,
                 QEnums.CONTENT_VISIBILITY_WARN_POST].includes(postContentVisibility) ||
               showWarnedPost
    }

    function getMuteText() {
        switch (postMuted) {
        case QEnums.MUTED_POST_AUTHOR:
            return qsTr("You muted this account")
        case QEnums.MUTED_POST_WORDS:
            return qsTr("Post has muted words")
        }

        return qsTr("Muted post")
    }

    function getPostFontSize() {
        if (!root.getSkywalker().getUserSettings().giantEmojis)
            return guiSettings.scaledFont(1)

        return onlyEmojisPost() ?
                    guiSettings.scaledFont(UnicodeFonts.graphemeLength(postPlainText) === 1 ? 9 : 3) :
                    guiSettings.scaledFont(1)
    }

    function onlyEmojisPost() {
        if (!postPlainText)
            return false

        if (UnicodeFonts.graphemeLength(postPlainText) > 5)
            return false

        return UnicodeFonts.onlyEmojis(postPlainText)
    }

    function mustShowLangauges() {
        return root.getSkywalker().getUserSettings().getShowLanguageTags()
    }

    function showLanguageLabels() {
        if (postLanguageLabels.length > 0 && mustShowLangauges()) {
            languageLabelsLoader.setSource("LanguageLabels.qml", {
                                               languageLabels: postLanguageLabels,
                                               parentWidth: width })
        }
    }

    function showContentLabels() {
        if (postContentLabels.length > 0) {
            contentLabelsLoader.setSource("ContentLabels.qml", {
                                        contentLabels: postContentLabels,
                                        filteredContentLabel: postBody.filteredContentLabel,
                                        contentAuthorDid: postAuthor.did,
                                        parentWidth: width})
        }
    }

    // Inital code loaded component from the QML files using the number of images
    // to create the file name
    // let qmlFile = `ImagePreview${(postImages.length)}.qml`
    // However when the bodyBackgroundColor changes, e.g. the user changes the
    // background, then the property maskColor did not automatically change too.
    // with explicit components it does.
    Component {
        id: images1Component

        ImagePreview1 {
            images: postImages
            maskColor: bodyBackgroundColor
            contentVisibility: postContentVisibility
            contentWarning: postContentWarning
            contentLabeler: postContentLabeler
            swipeMode: postBody.swipeMode

            onActivateSwipe: (imgIndex, previewImg) => postBody.activateSwipe(imgIndex, previewImg)
        }
    }

    Component {
        id: images2Component

        ImagePreview2 {
            images: postImages
            maskColor: bodyBackgroundColor
            contentVisibility: postContentVisibility
            contentWarning: postContentWarning
            contentLabeler: postContentLabeler
            swipeMode: postBody.swipeMode

            onActivateSwipe: (imgIndex, previewImg) => postBody.activateSwipe(imgIndex, previewImg)
        }
    }

    Component {
        id: images3Component

        ImagePreview3 {
            images: postImages
            maskColor: bodyBackgroundColor
            contentVisibility: postContentVisibility
            contentWarning: postContentWarning
            contentLabeler: postContentLabeler
            swipeMode: postBody.swipeMode

            onActivateSwipe: (imgIndex, previewImg) => postBody.activateSwipe(imgIndex, previewImg)
        }
    }

    Component {
        id: images4Component

        ImagePreview4 {
            images: postImages
            maskColor: bodyBackgroundColor
            contentVisibility: postContentVisibility
            contentWarning: postContentWarning
            contentLabeler: postContentLabeler
            swipeMode: postBody.swipeMode

            onActivateSwipe: (imgIndex, previewImg) => postBody.activateSwipe(imgIndex, previewImg)
        }
    }

    Component {
        id: videoThumbnailComponent

        VideoThumbnail {
            width: Math.min(180 * 1.777, postBody.width)
            height: 180
            videoSource: postBody.postVideo.playlistUrl
        }
    }

    Component {
        id: videoViewComponent

        VideoView {
            id: videoViewItem

            videoView: postBody.postVideo
            contentVisibility: postContentVisibility
            contentWarning: postContentWarning
            contentLabeler: postContentLabeler
            backgroundColor: bodyBackgroundColor
            highlight: bodyBackgroundColor === guiSettings.postHighLightColor
            swipeMode: postBody.swipeMode

            onActivateSwipe: postBody.activateSwipe(0, null)
        }
    }

    Component {
        id: externalViewComponent

        ExternalView {
            userDid: postBody.userDid
            postExternal: postBody.postExternal
            contentVisibility: postContentVisibility
            contentWarning: postContentWarning
            contentLabeler: postContentLabeler
            highlight: bodyBackgroundColor === guiSettings.postHighLightColor
        }
    }

    Component {
        id: unknownEmbedComponent

        UnknownEmbedView {
            width: parent.width
            unknownEmbedType: postUnknownEmbedType
        }
    }

    function showPostAttachements() {
        showLanguageLabels()

        if (postImages.length > 0) {
            const compList = [images1Component, images2Component, images3Component, images4Component]
            mediaLoader.sourceComponent = compList[postImages.length - 1]
            mediaLoader.active = true
        }
        else if (postVideo) {
            if (isDraft) {
                mediaLoader.sourceComponent = videoThumbnailComponent
                mediaLoader.active = true
            }
            else {
                mediaLoader.sourceComponent = videoViewComponent
                mediaLoader.active = true
            }
        }
        else if (postExternal) {
            mediaLoader.sourceComponent = externalViewComponent
            mediaLoader.active = true
        }
        else if (postHasUnknownEmbed) {
            mediaLoader.sourceComponent = unknownEmbedComponent
            mediaLoader.active = true
        }

        showContentLabels()

        if (postRecord)
            showPostRecord()
        else if (postRecordWithMedia)
            showPostRecordWidthMedia()
    }

    function showPostRecord() {
        if (!showRecord)
            return

        // Cannot use a direct component here, because of cyclic dependency.
        // RecordView has a PostBody
        recordLoader.setSource("RecordView.qml", {
                                   userDid: postBody.userDid,
                                   record: postRecord,
                                   backgroundColor: bodyBackgroundColor,
                                   highlight: bodyBackgroundColor === guiSettings.postHighLightColor })
    }

    function showPostRecordWidthMedia() {
        recordLoader.setSource("RecordWithMediaView.qml", {
                                   userDid: postBody.userDid,
                                   record: postRecordWithMedia,
                                   backgroundColor: bodyBackgroundColor,
                                   contentVisibility: postContentVisibility,
                                   contentWarning: postContentWarning,
                                   contentLabeler: postContentLabeler,
                                   highlight: bodyBackgroundColor === guiSettings.postHighLightColor,
                                   isDraft: isDraft,
                                   swipeMode: postBody.swipeMode,
                                   showRecord: postBody.showRecord })
    }

    onBodyBackgroundColorChanged: {
        if (recordLoader.item) {
            recordLoader.item.backgroundColor = bodyBackgroundColor
            recordLoader.item.highlight = bodyBackgroundColor === guiSettings.postHighLightColor
        }
    }

    onPostRecordChanged: {
        if (postRecord)
            showPostRecord()
    }

    onPostRecordWithMediaChanged: {
        if (postRecordWithMedia)
            showPostRecordWidthMedia()
    }

    onVisibleChanged: {
        if (postBody.visible && !postBody.attachmentsInitialized)
            initAttachments()
    }

    onWidthChanged: {
        // The disaply of these labels depend on the width of the post body
        showLanguageLabels()
        showContentLabels()
    }

    function initAttachments() {
        if (postVisible())
            showPostAttachements()

        postBody.attachmentsInitialized = true
    }

    Component.onCompleted: {
        if (!postBody.visible)
            return

        initAttachments()
    }
}
