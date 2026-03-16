import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

Column {
    property string userDid
    property Skywalker skywalker: root.getSkywalker(userDid)
    required property basicprofile postAuthor
    required property string postText
    required property textmetainfo postTextMetaInfo
    required property list<language> postLanguageLabels
    required property int postContentVisibility // QEnums::PostContentVisibility
    required property string postContentWarning
    required property basicprofile postContentLabeler
    required property int postMuted // QEnums::MutedPostReason
    required property string postPlainText
    required property bool postIsThread
    required property bool postIsThreadReply
    property int initialShowMaxTextLines: 25
    property int maxTextLines: 10000
    property string bodyBackgroundColor: guiSettings.backgroundColor
    property bool showWarnedPost: false
    property bool mutePost: postMuted !== QEnums.MUTED_POST_NONE
    property string postHighlightColor: "transparent"
    property int textBottomPadding: 0
    readonly property bool showThreadIndicator: postIsThread && !postPlainText.includes(UnicodeFonts.THREAD_SYMBOL)
    readonly property bool replaceThreadIndicator: (postIsThread || postIsThreadReply) && !showThreadIndicator

    // The font-size is set to make sure the thread indicator is in normal text size when the
    // post is giant emoji only.
    // The <div> cause a line break if there is post text before. In an empty post no newline
    // is prepended.
    readonly property string displayText:
        (replaceThreadIndicator ? UnicodeFonts.turnLastThreadSymbolIntoLink(postText) : postText) +
        (showThreadIndicator ? `<a href="${UnicodeFonts.THREAD_LINK}" style="text-decoration: none; font-size: ${Application.font.pixelSize * guiSettings.fontScaleFactor}px">${UnicodeFonts.THREAD_SYMBOL}</a>` : "")

    signal unrollThread

    id: postBody

    Loader {
        width: parent.width
        Layout.fillWidth: true
        active: postVisible() && displayText && (postTextMetaInfo.isNull() || postTextMetaInfo.newLineCount >= initialShowMaxTextLines)

        sourceComponent: PostBodyLongText {
            postHighlightColor: postBody.postHighlightColor
            initialShowMaxLineCount: Math.min(maxTextLines, initialShowMaxTextLines)
            maximumLineCount: maxTextLines
            ellipsisBackgroundColor: postBody.bodyBackgroundColor
            font.pointSize: getPostFontSize()
            plainText: displayText
            bottomPadding: textBottomPadding

            LinkCatcher {
                z: parent.z - 1
                containingText: postPlainText
                userDid: postBody.userDid
                author: postAuthor

                onUnrollThread: postBody.unrollThread()
            }
        }
    }

    Loader {
        width: parent.width
        Layout.fillWidth: true
        active: postVisible() && displayText && !postTextMetaInfo.isNull() && postTextMetaInfo.newLineCount < initialShowMaxTextLines

        sourceComponent: PostBodyShortText {
            postHighlightColor: postBody.postHighlightColor
            postTextMetaInfo: postBody.postTextMetaInfo
            hasThreadIndicator: replaceThreadIndicator || showThreadIndicator
            maximumLineCount: maxTextLines
            font.pointSize: getPostFontSize()
            text: displayText
            bottomPadding: textBottomPadding

            LinkCatcher {
                z: parent.z - 1
                containingText: postPlainText
                userDid: postBody.userDid
                author: postAuthor

                onUnrollThread: postBody.unrollThread()
            }
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
                    onLinkActivated: skywalker.getDetailedProfile(postContentLabeler.did)
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
                    onLinkActivated: skywalker.getDetailedProfile(postContentLabeler.did)
                }
            }
        }
    }

    // Languages
    Loader {
        anchors.right: parent.right

        // NOTE: without an explicit height, the full image animation is off by this height ??
        height: active ? guiSettings.labelHeight + 5 : 0

        active: postLanguageLabels.length > 0 && mustShowLanguages() && postVisible()
        sourceComponent: LanguageLabels {
            parentWidth: postBody.width
            languageLabels: postLanguageLabels
        }
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
        if (!skywalker.getUserSettings().giantEmojis)
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

    function mustShowLanguages() {
        return skywalker.getUserSettings().getShowLanguageTags()
    }
}
