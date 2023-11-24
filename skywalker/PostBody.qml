import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Column {
    required property string postText
    required property list<imageview> postImages
    required property date postDateTime
    required property list<string> postContentLabels
    required property int postContentVisibility // QEnums::PostContentVisibility
    required property string postContentWarning
    required property bool postMuted
    property string postPlainText
    property var postExternal // externalview (var allows NULL)
    property var postRecord // recordview
    property var postRecordWithMedia // record_with_media_view
    property bool detailedView: false
    property int maxTextLines: 1000
    property bool showWarnedPost: false
    property bool mutePost: postMuted

    id: postBody

    Text {
        id: bodyText
        width: parent.width
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        maximumLineCount: maxTextLines
        elide: Text.ElideRight
        textFormat: Text.RichText
        color: guiSettings.textColor
        font.pointSize: getPostFontSize()
        text: postText
        bottomPadding: postImages.length > 0 || postExternal || postRecord ? 5 : 0
        visible: postVisible()

        onLinkActivated: (link) => {
            if (link.startsWith("did:")) {
                console.debug("MENTION", link)
                skywalker.getDetailedProfile(link)
            } else {
                console.debug("LINK:", link)
                root.openLink(link)
            }
        }
    }

    Row {
        width: parent.width
        spacing: 10

        SvgImage {
            id: imgIcon
            width: 30
            height: width
            color: Material.color(Material.Grey)
            svg: mutePost ? svgOutline.mute : svgOutline.hideVisibility
            visible: !postVisible()
        }

        // The content warning is shown then the post is not muted
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
            text: qsTr("You muted this account") + `<br><a href=\"show\" style=\"color: ${guiSettings.linkColor};\">` + qsTr("Show post") + "</a>"
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

    function getPostFontSize() {
        return onlyEmojisPost() ?
                    guiSettings.scaledFont(postUtils.graphemeLength(postPlainText) === 1 ? 9 : 3) :
                    guiSettings.scaledFont(1)
    }

    function onlyEmojisPost() {
        if (!postPlainText)
            return false

        if (postUtils.graphemeLength(postPlainText) > 5)
            return false

        return postUtils.onlyEmojis(postPlainText)
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
            component.createObject(postBody, {contentLabels: postContentLabels})
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

    Component.onCompleted: {
        if (!postBody.visible)
            return

        if (postVisible())
            showPostAttachements()

        if (detailedView)
            dateTimeComp.createObject(postBody)
    }
}
