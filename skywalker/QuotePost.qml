import QtQuick
import QtQuick.Layouts
import skywalker

Column {
    property basicprofile author
    property string postText
    property date postDateTime
    property string ellipsisBackgroundColor: guiSettings.backgroundColor
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

        Avatar {
            id: avatar
            width: 24
            //Layout.alignment: Qt.AlignTop
            avatarUrl: author.avatarUrl
            isModerator: author.associated.isLabeler

            onClicked: skywalker.getDetailedProfile(author.did)
        }

        PostHeader {
            Layout.fillWidth: true
            authorName: author.name
            authorHandle: author.handle
            postThreadType: QEnums.THREAD_NONE
            postIndexedSecondsAgo: (new Date() - postDateTime) / 1000
        }

        SvgButton {
            Layout.preferredWidth: 34
            Layout.preferredHeight: 34
            svg: svgOutline.close
            accessibleName: qsTr("remove quoted post")
            focusPolicy: Qt.NoFocus
            visible: showCloseButton
            onClicked: closeClicked()
        }
    }

    PostBody {
        width: parent.width - 20
        postAuthor: author
        postText: quoteColumn.postText
        postImages: []
        postLanguageLabels: []
        postContentLabels: []
        postContentVisibility: QEnums.CONTENT_VISIBILITY_SHOW
        postContentWarning: ""
        postMuted: QEnums.MUTED_POST_NONE
        postDateTime: postDateTime
        maxTextLines: 5
        ellipsisBackgroundColor: quoteColumn.ellipsisBackgroundColor
    }

    AccessibilityUtils {
        id: accessibilityUtils
    }

    GuiSettings {
        id: guiSettings
    }
}
