import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

Column {
    property string postCid
    required property basicprofile postAuthor
    required property string postText
    required property list<imageview> postImages
    required property date postDateTime
    required property list<language> postLanguageLabels
    required property list<contentlabel> postContentLabels
    required property int postContentVisibility // QEnums::PostContentVisibility
    required property string postContentWarning
    required property int postMuted // QEnums::MutedPostReason
    property string postPlainText
    property var postVideo // videoView
    property var postExternal // externalview (var allows NULL)
    property var postRecord // recordview
    property var postRecordWithMedia // record_with_media_view
    property bool detailedView: false
    property int maxTextLines: 1000
    property string bodyBackgroundColor: guiSettings.backgroundColor
    property string borderColor: guiSettings.borderColor
    property bool showWarnedPost: false
    property bool mutePost: postMuted !== QEnums.MUTED_POST_NONE
    property bool attachmentsInitialized: false
    property string postHighlightColor: "transparent"
    property bool isDraft: false
    property bool isVideoFeed: false

    id: postBody

    SkyCleanedText {
        id: bodyText
        width: parent.width
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        initialShowMaxLineCount: Math.min(maxTextLines, 25)
        maximumLineCount: maxTextLines
        ellipsisBackgroundColor: postBody.bodyBackgroundColor
        elide: Text.ElideRight
        textFormat: Text.RichText
        color: guiSettings.textColor
        font.pointSize: getPostFontSize()
        plainText: postText
        bottomPadding: postImages.length > 0 || postVideo || postExternal || postRecord || postRecordWithMedia ? 5 : 0
        visible: postVisible() && postText

        onLinkActivated: (link) => root.openLink(link)

        Accessible.ignored: true

        Rectangle {
            anchors.fill: parent
            z: parent.z - 1
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
            Text {
                id: warnText
                width: parent.width
                Layout.fillWidth: true
                anchors.verticalCenter: parent.verticalCenter
                wrapMode: Text.Wrap
                elide: Text.ElideRight
                textFormat: Text.RichText
                color: Material.color(Material.Grey)
                text: postContentWarning + `<br><a href=\"show\" style=\"color: ${guiSettings.linkColor};\">` + qsTr("Show post") + "</a>"
                visible: postContentVisibility === QEnums.CONTENT_VISIBILITY_WARN_POST && !showWarnedPost && !mutePost
                onLinkActivated: {
                    showWarnedPost = true

                    if (postVisible())
                        showPostAttachements()
                }
            }

            // If the post is muted, then this takes precendence over the content warning
            Text {
                id: mutedText
                width: parent.width
                Layout.fillWidth: true
                anchors.verticalCenter: parent.verticalCenter
                wrapMode: Text.Wrap
                elide: Text.ElideRight
                textFormat: Text.RichText
                color: Material.color(Material.Grey)
                text: getMuteText() + `<br><a href=\"show\" style=\"color: ${guiSettings.linkColor};\">` + qsTr("Show post") + "</a>"
                visible: mutePost && postContentVisibility !== QEnums.CONTENT_VISIBILITY_HIDE_POST
                onLinkActivated: {
                    mutePost = false

                    // The post may still not be visible due to content filtering
                    if (postVisible())
                        showPostAttachements()
                }
            }

            // If a post is hidden then this text will show no matter whether the post is muted
            Text {
                id: hideText
                width: parent.width
                Layout.fillWidth: true
                anchors.verticalCenter: parent.verticalCenter
                wrapMode: Text.Wrap
                elide: Text.ElideRight
                textFormat: Text.RichText
                color: Material.color(Material.Grey)
                text: postContentWarning
                visible: postContentVisibility === QEnums.CONTENT_VISIBILITY_HIDE_POST
            }
        }
    }

    Loader {
        id: languageLabelsLoader
        anchors.right: parent.right
        visible: status == Loader.Ready
    }

    Loader {
        id: mediaLoader
        width: parent.width
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

        if (postRecordWithMedia && recordLoader.item)
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
                                        contentAuthorDid: postAuthor.did,
                                        parentWidth: width})
        }
    }

    function showPostAttachements() {
        showLanguageLabels()

        if (postImages.length > 0) {
            let qmlFile = `ImagePreview${(postImages.length)}.qml`
            mediaLoader.setSource(qmlFile, {
                                      images: postImages,
                                      maskColor: bodyBackgroundColor,
                                      contentVisibility: postContentVisibility,
                                      contentWarning: postContentWarning })
        }
        else if (postVideo) {
            if (isDraft) {
                mediaLoader.setSource("VideoThumbnail.qml", {
                                        width: Math.min(180 * 1.777, postBody.width),
                                        height: 180,
                                        videoSource: postBody.postVideo.playlistUrl });
            }
            else {
                mediaLoader.setSource("VideoView.qml", {
                                          postCid: postBody.postCid,
                                          videoView: postBody.postVideo,
                                          contentVisibility: postContentVisibility,
                                          contentWarning: postContentWarning,
                                          backgroundColor: bodyBackgroundColor,
                                          highlight: bodyBackgroundColor === guiSettings.postHighLightColor,
                                          isVideoFeed: postBody.isVideoFeed })
            }
        }
        else if (postExternal) {
            mediaLoader.setSource("ExternalView.qml", {
                                      postExternal: postBody.postExternal,
                                      contentVisibility: postContentVisibility,
                                      contentWarning: postContentWarning,
                                      highlight: bodyBackgroundColor === guiSettings.postHighLightColor })
        }

        showContentLabels()

        if (postRecord)
            showPostRecord()
        else if (postRecordWithMedia)
            showPostRecordWidthMedia()
    }

    function showPostRecord() {
        recordLoader.setSource("RecordView.qml", {
                                   record: postRecord,
                                   backgroundColor: bodyBackgroundColor,
                                   highlight: bodyBackgroundColor === guiSettings.postHighLightColor })
    }

    function showPostRecordWidthMedia() {
        recordLoader.setSource("RecordWithMediaView.qml", {
                                   record: postRecordWithMedia,
                                   backgroundColor: bodyBackgroundColor,
                                   contentVisibility: postContentVisibility,
                                   contentWarning: postContentWarning,
                                   highlight: bodyBackgroundColor === guiSettings.postHighLightColor,
                                   isVideoFeed: isVideoFeed })
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
