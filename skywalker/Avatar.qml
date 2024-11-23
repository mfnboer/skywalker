import QtQuick
import skywalker

Item {
    property basicprofile author
    property int radius: width / 2
    property SvgImage unknownSvg: SvgFilled.unknownAvatar
    readonly property int contentVisibility: root.getSkywalker().getContentVisibility(author.labels)
    property bool showWarnedMedia: false
    readonly property bool showThumb: width < 90 // from bsky client code
    property string avatarUrl: !contentVisible() ? "" : (showThumb ? author.avatarThumbUrl : author.avatarUrl)

    signal clicked
    signal pressAndHold

    id: avatarItem
    height: width

    RoundedFrame {
        id: avatarFrame
        objectToRound: avatarImg
        width: parent.width
        height: parent.height
        radius: parent.radius
        visible: avatarUrl && avatarImg.status === Image.Ready

        ImageAutoRetry {
            id: avatarImg
            width: parent.width
            source: avatarUrl
            fillMode: Image.PreserveAspectFit
            maxRetry: 60
        }
    }
    Rectangle {
        width: parent.width
        height: parent.height
        radius: parent.radius
        color: guiSettings.avatarDefaultColor
        visible: !avatarFrame.visible

        SkySvg {
            width: parent.width
            height: parent.height
            color: "white"
            svg: author.associated.isLabeler ? SvgFilled.moderator : avatarItem.unknownSvg
        }
    }
    MouseArea {
        anchors.fill: parent
        onClicked: {
            console.debug("Avatar clicked");
            avatarItem.clicked()
        }
        onPressAndHold: {
            console.debug("Avatar press and hold")
            avatarItem.pressAndHold()
        }
    }

    ModeratorIcon {
        width: parent.width * 0.6
        visible: author.associated.isLabeler
    }


    function contentVisible() {
        return showWarnedMedia || guiSettings.contentVisible(author)
    }
}
