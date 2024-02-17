import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Column {
    required property basicprofile postAuthor
    required property string postText
    required property list<imageview> postImages
    required property date postDateTime
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

    id: postBody

    SkyCleanedText {
        id: bodyText
        width: parent.width
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        maximumLineCount: maxTextLines
        ellipsisBackgroundColor: postBody.ellipsisBackgroundColor
        elide: Text.ElideRight
        textFormat: Text.RichText
        color: guiSettings.textColor
        font.pointSize: getPostFontSize()
        plainText: postText
        bottomPadding: postImages.length > 0 || postExternal || postRecord ? 5 : 0
        visible: postVisible()

        onLinkActivated: (link) => root.openLink(link)

        Accessible.ignored: true
    }

    Row {
        width: parent.width
        spacing: 10

        SvgImage {
            id: imgIcon
            width: 30
            height: width
            color: Material.color(Material.Grey)
            svg: getIcon()
            visible: !postVisible()

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

    Component {
        id: dateTimeComp
        Text {
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

    function showPostAttachements() {
        if (postImages.length > 0) {
            let qmlFile = `ImagePreview${(postImages.length)}.qml`
            let component = Qt.createComponent(qmlFile)
            component.createObject(postBody, {images: postImages,
                                              contentVisibility: postContentVisibility,
                                              contentWarning: postContentWarning})
        }

        if (postContentLabels.length > 0) {
            let component = Qt.createComponent("ContentLabels.qml")
            component.createObject(postBody, {contentLabels: postContentLabels, contentAuthorDid: postAuthor.did})
        }

        if (postExternal) {
            let component = Qt.createComponent("ExternalView.qml")
            component.createObject(postBody, {postExternal: postBody.postExternal,
                                              contentVisibility: postContentVisibility,
                                              contentWarning: postContentWarning})
        }

        if (postRecord) {
            let component = Qt.createComponent("RecordView.qml")
            component.createObject(postBody, {record: postRecord})
        }

        if (postRecordWithMedia) {
            let component = Qt.createComponent("RecordWithMediaView.qml")
            component.createObject(postBody, {record: postRecordWithMedia,
                                              contentVisibility: postContentVisibility,
                                              contentWarning: postContentWarning})
        }
    }

    onVisibleChanged: {
        if (postBody.visible && !postBody.attachmentsInitialized)
            initAttachments()
    }

    function initAttachments() {
        if (postVisible())
            showPostAttachements()

        if (detailedView)
            dateTimeComp.createObject(postBody)

        postBody.attachmentsInitialized = true
    }

    Component.onCompleted: {
        if (!postBody.visible)
            return

        initAttachments()
    }
}
