import QtQuick
import QtQuick.Controls
import skywalker

Item {
    property basicprofile author
    property int radius: width / 2
    property SvgImage unknownSvg: SvgFilled.unknownAvatar
    readonly property int contentVisibility: root.getSkywalker().getContentVisibility(author.labels)
    property bool showWarnedMedia: false
    readonly property bool showThumb: width < 90 // from bsky client code
    property string avatarUrl: !contentVisible() ? "" : (showThumb ? author.avatarThumbUrl : author.avatarUrl)
    property bool showModeratorIcon: true

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
            height: parent.height
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
    Loader {
        active: author.actorStatus.isActive
        sourceComponent: Rectangle {
            id: liveRect
            width: avatarItem.width
            height: avatarItem.height
            radius: avatarItem.radius
            color: "transparent"
            border.color: "red"
            border.width: 2

            Label {
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.bottom
                padding: 1
                color: "white"
                font.bold: true
                font.pointSize: guiSettings.scaledFont(5/8 / 37 * avatarItem.width)
                text: qsTr("LIVE")

                background: Rectangle {
                    radius: 3
                    color: liveRect.border.color
                }
            }
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
