import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Item {
    property basicprofile author
    property int radius: width / 2
    property SvgImage unknownSvg: SvgFilled.unknownAvatar
    property var skywalker: root.getSkywalker()
    readonly property int contentVisibility: skywalker.getContentVisibility(author.labels)
    property bool showWarnedMedia: false
    readonly property bool showThumb: width < 90 // from bsky client code
    property string avatarUrl: !contentVisible() ? "" : (showThumb ? author.avatarThumbUrl : author.avatarUrl)
    property bool showModeratorIcon: true
    readonly property ActivityStatus activityStatus: skywalker.getFollowsActivityStore().getActivityStatus(author.did)
    readonly property date lastActive: activityStatus.lastActive
    readonly property bool isActive: activityStatus.active

    signal clicked
    signal pressAndHold

    id: avatarItem
    height: width + liveLoader.getExtraHeight()
    Layout.preferredHeight: Layout.preferredWidth + liveLoader.getExtraHeight()

    RoundedFrame {
        id: avatarFrame
        objectToRound: avatarImg
        width: parent.width
        height: width
        radius: parent.radius
        visible: avatarUrl && avatarImg.status === Image.Ready

        ImageAutoRetry {
            id: avatarImg
            width: parent.width
            height: width
            source: avatarUrl
            sourceSize.width: width * Screen.devicePixelRatio
            sourceSize.height: height * Screen.devicePixelRatio
            fillMode: Image.PreserveAspectFit
            maxRetry: 60
            indicateLoading: false
        }
    }
    Rectangle {
        width: parent.width
        height: width
        radius: parent.radius
        color: guiSettings.avatarDefaultColor
        visible: !avatarFrame.visible

        SkySvg {
            width: parent.width
            height: width
            color: "white"
            svg: author.associated.isLabeler ? SvgFilled.moderator : avatarItem.unknownSvg
        }
    }
    Loader {
        id: liveLoader
        active: author.actorStatus.isActive

        sourceComponent: Rectangle {
            id: liveRect
            width: avatarItem.width
            height: width
            radius: avatarItem.radius
            color: "transparent"
            border.color: guiSettings.liveColor
            border.width: 2

            LiveLabel {
                id: liveLabel
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.bottom
                font.pointSize: avatarItem.width > 0 ? guiSettings.scaledFont(5/8 / 37 * avatarItem.width) : guiSettings.scaledFont(5/8)
            }

            function getLiveLabelHeight() {
                return liveLabel.height
            }
        }

        function getExtraHeight() {
            return item ? item.getLiveLabelHeight() / 2 : 0
        }
    }

    Loader {
        id: activeLoader
        active: avatarItem.isActive

        sourceComponent: Rectangle {
            x: avatarItem.width - width
            y: avatarItem.height - height
            width: avatarItem.width * 0.15
            height: width
            radius: width / 2
            color: guiSettings.activeColor
            border.color: guiSettings.activeBorderColor
            border.width: 1
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
        visible: author.associated.isLabeler && showModeratorIcon
    }

    function contentVisible() {
        return showWarnedMedia || guiSettings.contentVisible(author)
    }

    function getImage() {
        return avatarImg
    }
}
