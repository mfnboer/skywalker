import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

// Copy from PostBody without record, recordWithMedia
// Could not use PostBody inside records as that would give a cyclic dependency
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
    property bool detailedView: false
    property int initialShowMaxTextLines: 25
    property int maxTextLines: 10000
    property string bodyBackgroundColor: guiSettings.backgroundColor
    property string borderColor: guiSettings.borderColor
    property bool showWarnedPost: false
    property bool mutePost: postMuted !== QEnums.MUTED_POST_NONE
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
        (showThreadIndicator ? `<a href="${UnicodeFonts.THREAD_LINK}" style="text-decoration: none; font-size: ${Application.font.pixelSize * guiSettings.fontScaleFactor}px">${UnicodeFonts.THREAD_SYMBOL}</a>` : "")

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
        bottomPadding: postImages.length > 0 || postVideo || postExternal || postHasUnknownEmbed ? 5 : 0
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

                AccessibleText {
                    width: parent.width
                    wrapMode: Text.Wrap
                    elide: Text.ElideRight
                    textFormat: Text.RichText
                    color: Material.color(Material.Grey)
                    text: postContentWarning
                }
                AccessibleText {
                    width: parent.width
                    elide: Text.ElideRight
                    textFormat: Text.RichText
                    font.pointSize: guiSettings.scaledFont(7/8)
                    font.italic: true
                    text: qsTr(`<a href="link" style="color: ${guiSettings.linkColor}; text-decoration: none">@${postContentLabeler.handle}</a>`)
                    visible: !postContentLabeler.isNull()
                    onLinkActivated: root.getSkywalker().getDetailedProfile(postContentLabeler.did)
                }
                AccessibleText {
                    topPadding: 20
                    width: parent.width
                    elide: Text.ElideRight
                    textFormat: Text.RichText
                    text: `<a href=\"show\" style=\"color: ${guiSettings.linkColor};\">` + qsTr("Show post") + "</a>"
                    onLinkActivated: showWarnedPost = true
                }
            }

            // If the post is muted, then this takes precendence over the content warning
            Column {
                width: parent.width
                anchors.verticalCenter: parent.verticalCenter
                visible: mutePost && postContentVisibility !== QEnums.CONTENT_VISIBILITY_HIDE_POST

                AccessibleText {
                    width: parent.width
                    wrapMode: Text.Wrap
                    elide: Text.ElideRight
                    textFormat: Text.RichText
                    color: Material.color(Material.Grey)
                    text: getMuteText()
                }
                AccessibleText {
                    topPadding: 20
                    width: parent.width
                    elide: Text.ElideRight
                    textFormat: Text.RichText
                    text: `<a href=\"show\" style=\"color: ${guiSettings.linkColor};\">` + qsTr("Show post") + "</a>"
                    onLinkActivated: mutePost = false
                }
            }

            // If a post is hidden then this text will show no matter whether the post is muted
            Column {
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width
                visible: postContentVisibility === QEnums.CONTENT_VISIBILITY_HIDE_POST

                AccessibleText {
                    width: parent.width
                    wrapMode: Text.Wrap
                    elide: Text.ElideRight
                    textFormat: Text.RichText
                    color: Material.color(Material.Grey)
                    text: postContentWarning
                }
                AccessibleText {
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

    // Languages
    Loader {
        anchors.right: parent.right
        active: postLanguageLabels.length > 0 && mustShowLangauges() && postVisible()
        sourceComponent: LanguageLabels {
            parentWidth: parent.width
            languageLabels: postLanguageLabels
        }
    }

    // Images
    Loader {
        x: swipeMode ? -margin : 0
        width: parent.width + (swipeMode ? 2 * margin : 0)
        active: postImages.length === 1 && postVisible()
        sourceComponent: images1Component
    }
    Loader {
        x: swipeMode ? -margin : 0
        width: parent.width + (swipeMode ? 2 * margin : 0)
        active: postImages.length === 2 && postVisible()
        sourceComponent: images2Component
    }
    Loader {
        x: swipeMode ? -margin : 0
        width: parent.width + (swipeMode ? 2 * margin : 0)
        active: postImages.length === 3 && postVisible()
        sourceComponent: images3Component
    }
    Loader {
        x: swipeMode ? -margin : 0
        width: parent.width + (swipeMode ? 2 * margin : 0)
        active: postImages.length === 4 && postVisible()
        sourceComponent: images4Component
    }

    // Video
    // HACK: somehow video leaves 1 empty pixel at each side. Add 2 pixels to fix it.
    Loader {
        id: videoLoader
        x: swipeMode ? -margin - 1 : 0
        width: parent.width + (swipeMode ? 2 * margin + 2 : 0)
        active: Boolean(postVideo) && postVisible()
        sourceComponent: isDraft ? videoThumbnailComponent : videoViewComponent
    }

    // External
    Loader {
        width: parent.width
        active: Boolean(postExternal) && postVisible()
        sourceComponent: externalViewComponent
    }

    // Unknown embed
    Loader {
        width: parent.width
        active: postHasUnknownEmbed && postVisible()
        sourceComponent: unknownEmbedComponent
    }

    Loader {
        anchors.right: parent.right
        active: postContentLabels.length > 0 && postVisible()
        sourceComponent: ContentLabels {
            parentWidth: parent.width
            contentLabels: postContentLabels
            filteredContentLabel: postBody.filteredContentLabel
            contentAuthorDid: postAuthor.did
        }
    }

    Loader {
        id: dateTimeLoader
        width: parent.width
        active: detailedView && postVisible()
        sourceComponent: AccessibleText {
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
        if (videoLoader.item)
            videoLoader.item.pause() // qmllint disable missing-property
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

            onActivateSwipe: (imgIndex, previewImg) => postBody.activateSwipe(imgIndex, previewImg)
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
}
