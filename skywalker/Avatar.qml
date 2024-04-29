import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import skywalker

Item {
    property string avatarUrl
    property bool isModerator: false
    property int radius: width / 2
    property svgimage unknownSvg: svgFilled.unknownAvatar

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
        visible: avatarItem.avatarUrl && avatarImg.status === Image.Ready

        ImageAutoRetry {
            id: avatarImg
            width: parent.width
            source: avatarItem.avatarUrl
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

        SvgImage {
            width: parent.width
            height: parent.height
            color: "white"
            svg: avatarItem.unknownSvg
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
        width: parent.width * 0.3
        visible: isModerator
    }

    GuiSettings {
        id: guiSettings
    }
}
