import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Column {
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
    property var postExternal // externalview (var allows NULL)
    property var postRecord // recordview
    property var postRecordWithMedia // record_with_media_view
    property bool detailedView: false
    property int maxTextLines: 1000
    property string ellipsisBackgroundColor: guiSettings.backgroundColor
    property bool showWarnedPost: false
    property bool mutePost: postMuted !== QEnums.MUTED_POST_NONE
    property bool attachmentsInitialized: false
    property string postHighlightColor: "transparent"

    // Dynamic objects
    property bool isPooled: false

    id: postBody

    SkyCleanedText {
        id: bodyText
        width: parent.width
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        initialShowMaxLineCount: Math.min(maxTextLines, 25)
        maximumLineCount: maxTextLines
        ellipsisBackgroundColor: postBody.ellipsisBackgroundColor
        elide: Text.ElideRight
        textFormat: Text.RichText
        color: guiSettings.textColor
        font.pointSize: getPostFontSize()
        plainText: postText
        bottomPadding: postImages.length > 0 || postExternal || postRecord ? 5 : 0
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

            SvgImage {
                id: imgIcon
                width: 30
                height: width
                color: Material.color(Material.Grey)
                svg: getIcon()

                function getIcon() {
                    if (!mutePost)
                        return svgOutline.hideVisibility

                    switch (postMuted) {
                    case QEnums.MUTED_POST_AUTHOR:
                        return svgOutline.mute
                    case QEnums.MUTED_POST_WORDS:
                        return svgOutline.mutedWords
                    }

                    return svgOutline.hideVisibility
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
        id: imageLoader
        width: parent.width
        visible: status == Loader.Ready
    }

    Loader {
        id: externalLoader
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
        visible: status == Loader.Ready
    }

    Loader {
        id: recordWithMediaLoader
        width: parent.width
        visible: status == Loader.Ready
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
            text: postDateTime.toLocaleString(Qt.locale(), Locale.LongFormat)
            font.pointSize: guiSettings.scaledFont(7/8)
        }
    }

    GuiSettings {
        id: guiSettings
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
        return onlyEmojisPost() ?
                    guiSettings.scaledFont(unicodeFonts.graphemeLength(postPlainText) === 1 ? 9 : 3) :
                    guiSettings.scaledFont(1)
    }

    function onlyEmojisPost() {
        if (!postPlainText)
            return false

        if (unicodeFonts.graphemeLength(postPlainText) > 5)
            return false

        return unicodeFonts.onlyEmojis(postPlainText)
    }

    function mustShowLangaugess() {
        return root.getSkywalker().getUserSettings().getShowLanguageTags()
    }

    function showPostAttachements() {
        if (postLanguageLabels.length > 0 && mustShowLangaugess()) {
            languageLabelsLoader.setSource("LanguageLabels.qml", {
                                               languageLabels: postLanguageLabels,
                                               parentWidth: parent.width })
        }

        if (postImages.length > 0) {
            let qmlFile = `ImagePreview${(postImages.length)}.qml`
            imageLoader.setSource(qmlFile, {
                                      images: postImages,
                                      contentVisibility: postContentVisibility,
                                      contentWarning: postContentWarning })
        }

        if (postExternal) {
            externalLoader.setSource("ExternalView.qml", {
                                        postExternal: postBody.postExternal,
                                        contentVisibility: postContentVisibility,
                                        contentWarning: postContentWarning })
        }

        if (postContentLabels.length > 0) {
            contentLabelsLoader.setSource("ContentLabels.qml", {
                                        contentLabels: postContentLabels,
                                        contentAuthorDid: postAuthor.did,
                                        parentWidth: parent.width})
        }

        if (postRecord)
            recordLoader.setSource("RecordView.qml", {record: postRecord})

        if (postRecordWithMedia) {
            recordWithMediaLoader.setSource("RecordWithMediaView.qml", {
                                                record: postRecordWithMedia,
                                                contentVisibility: postContentVisibility,
                                                contentWarning: postContentWarning })
        }
    }

    onVisibleChanged: {
        if (isPooled)
            return

        if (postBody.visible && !postBody.attachmentsInitialized)
            initAttachments()
    }

    function pooled() {
        languageLabelsLoader.source = ""
        imageLoader.source = ""
        externalLoader.source = ""
        contentLabelsLoader.source = ""
        recordLoader.source = ""
        recordWithMediaLoader.source = ""
        attachmentsInitialized = false

        showWarnedPost = false
        bodyText.capLineCount = bodyText.initialShowMaxLineCount
        isPooled = true
    }

    function reused() {
        isPooled = false
        initAttachments()
    }

    function initAttachments() {
        if (isPooled) {
            console.debug("IS POOLED!!!")
            return
        }

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
