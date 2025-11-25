import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Column {
    property string userDid
    property basicprofile author
    property string postText
    property date postDateTime
    property string postBackgroundColor: guiSettings.backgroundColor
    property bool showCloseButton: false

    signal closeClicked

    id: quoteColumn
    padding: 10

    Accessible.role: Accessible.StaticText
    Accessible.name: getSpeech()

    function getSpeech() {
        const time = accessibilityUtils.getTimeSpeech(postDateTime)
        const speech = `${author.name}\n\n${time}\n\n${postText}`
        return speech
    }

    RowLayout {
        width: parent.width - 20

        PostHeaderWithAvatar {
            Layout.fillWidth: true
            userDid:quoteColumn.userDid
            author: quoteColumn.author
            postIndexedSecondsAgo: (new Date() - postDateTime) / 1000
        }

        SvgButton {
            Layout.preferredWidth: 34
            Layout.preferredHeight: 34
            svg: SvgOutline.close
            accessibleName: qsTr("remove quoted post")
            focusPolicy: Qt.NoFocus
            visible: showCloseButton
            onClicked: closeClicked()
        }
    }

    Flickable {
        id: flick
        width: parent.width - 20
        height: Math.min(contentHeight, guiSettings.appFontHeight * 5)
        clip: true
        contentHeight: contentItem.childrenRect.height
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: SkyScrollBarVertical {}

        PostBody {
            width: parent.width - (height > guiSettings.appFontHeight * 5 ? 10 : 0)
            userDid: quoteColumn.userDid
            postAuthor: author
            postText: quoteColumn.postText
            postPlainText: quoteColumn.postText
            postHasUnknownEmbed: false
            postUnknownEmbedType: ""
            postImages: []
            postLanguageLabels: []
            postContentLabels: []
            postContentVisibility: QEnums.CONTENT_VISIBILITY_SHOW
            postContentWarning: ""
            postContentLabeler: accessibilityUtils.nullAuthor
            postMuted: QEnums.MUTED_POST_NONE
            postIsThread: false
            postIsThreadReply: false
            postDateTime: postDateTime
            initialShowMaxTextLines: maxTextLines
            bodyBackgroundColor: quoteColumn.postBackgroundColor
        }
    }

    AccessibilityUtils {
        id: accessibilityUtils
    }

}
