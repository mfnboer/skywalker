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
    property string postPlainText
    property var postExternal // externalview (var allows NULL)
    property var postRecord // recordview
    property var postRecordWithMedia // record_with_media_view
    property bool detailedView: false
    property int maxTextLines: 1000
    property bool showWarnedPost: false

    id: postBody

    Text {
        id: bodyText
        width: parent.width
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        maximumLineCount: maxTextLines
        elide: Text.ElideRight
        textFormat: Text.RichText
        color: Material.foreground
        font.pointSize: getPostFontSize()
        text: postText
        bottomPadding: postImages.length > 0 || postExternal || postRecord ? 5 : 0
        visible: postVisible()

        onLinkActivated: (link) => {
            if (link.startsWith("did:")) {
                console.debug("MENTION", link)
                skywalker.getDetailedProfile(link)
            } else {
                Qt.openUrlExternally(link)
            }
        }
    }

    Text {
        id: warnText
        width: parent.width
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        elide: Text.ElideRight
        textFormat: Text.RichText
        color: "red"
        // TODO: icon
        text: qsTr("WARNING") + ": " + postContentWarning + "<br><a href=\"show\">" + qsTr("Show post") + "</a>"
        visible: postContentVisibility === QEnums.CONTENT_VISIBILITY_WARN_POST && !showWarnedPost
        onLinkActivated: {
            showWarnedPost = true
            showPostAttachements()
        }
    }

    Text {
        id: hideText
        width: parent.width
        Layout.fillWidth: true
        wrapMode: Text.Wrap
        elide: Text.ElideRight
        textFormat: Text.RichText
        color: "red"
        // TODO: icon
        text: qsTr("HIDDEN") + ": " + postContentWarning
        visible: postContentVisibility === QEnums.CONTENT_VISIBILITY_HIDE_POST
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
            component.createObject(postBody, {postExternal: postBody.postExternal})
        }

        if (postRecord) {
            let component = Qt.createComponent("RecordView.qml")
            component.createObject(postBody, {record: postRecord})
        }

        if (postRecordWithMedia) {
            let component = Qt.createComponent("RecordWithMediaView.qml")
            component.createObject(postBody, {record: postRecordWithMedia})
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
