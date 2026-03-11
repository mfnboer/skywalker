import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Item {
    property string userDid
    property basicprofile author
    property int radius: width / 2
    property SvgImage unknownSvg: SvgFilled.unknownAvatar
    property Skywalker skywalker: root.getSkywalker(userDid)
    property var userSettings: skywalker.getUserSettings()
    property bool showWarnedMedia: false
    readonly property bool showThumb: width < 90 // from bsky client code
    property string avatarUrl: !contentVisible() ? "" : (showThumb ? author.avatarThumbUrl : author.avatarUrl)
    property bool showModeratorIcon: true
    readonly property ActivityStatus activityStatus: skywalker.getFollowsActivityStore().getActivityStatus(author)
    readonly property date lastActive: activityStatus ? activityStatus.lastActive : new Date(undefined)
    readonly property bool isActive: activityStatus ? activityStatus.active : false
    property bool showFollowingStatus: true
    property bool authorIsLive: author.actorStatus.isActive

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
    Loader {
        width: parent.width
        height: width
        active: avatarImg.status !== Image.Ready || !avatarUrl

        sourceComponent: Rectangle {
            radius: avatarItem.radius
            color: guiSettings.avatarDefaultColor

            SkySvg {
                width: parent.width
                height: width
                color: "white"
                svg: author.associated.isLabeler ? SvgFilled.moderator : avatarItem.unknownSvg
            }
        }
    }
    Loader {
        id: liveLoader
        active: authorIsLive

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
        readonly property bool activeStatus: avatarItem.isActive && userSettings.showFollowsActiveStatus && showFollowingStatus
        readonly property bool followsStatus: author.viewer.following && userSettings.showFollowsStatus && showFollowingStatus

        id: followingLoader
        active: activeStatus || followsStatus
        //active: avatarItem.isActive && userSettings.showFollowsActiveStatus && showActivityStatus

        sourceComponent: Rectangle {
            x: avatarItem.width - width
            y: avatarItem.height - height
            width: avatarItem.width * 0.15
            height: width
            radius: width / 2
            color: followingLoader.activeStatus ? guiSettings.activeColor : guiSettings.accentColor
            border.color: followingLoader.activeStatus ? guiSettings.activeBorderColor : guiSettings.accentColor
            border.width: 1
        }
    }

    SkyMouseArea {
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

    Loader {
        width: parent.width
        height: parent.height
        active: author.associated.isLabeler && showModeratorIcon
        sourceComponent: Item {
            ModeratorIcon {
                x: parent.width - width * 0.8
                y: height - height * 0.2
                width: parent.width * 0.6
                height: width
            }
        }
    }

    function contentVisible() {
        return showWarnedMedia || guiSettings.contentVisible(author, userDid)
    }

    function getImage() {
        return avatarImg
    }
}
