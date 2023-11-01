import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import skywalker

Item {
    property string avatarUrl
    signal clicked
    signal pressAndHold

    id: avatarItem
    height: width

    RoundedFrame {
        id: avatarFrame
        objectToRound: avatarImg
        width: parent.width
        height: parent.height
        radius: parent.width / 2
        visible: avatarItem.avatarUrl && avatarImg.status === Image.Ready

        ImageAutoRetry {
            id: avatarImg
            width: parent.width
            source: avatarItem.avatarUrl
            fillMode: Image.PreserveAspectFit
        }
    }
    Rectangle {
        width: parent.width
        height: parent.height
        radius: height / 2
        color: guiSettings.buttonColor
        visible: !avatarFrame.visible

        SvgImage {
            width: parent.width
            height: parent.height
            color: guiSettings.buttonTextColor
            svg: svgFilled.unknownAvatar
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

    GuiSettings {
        id: guiSettings
    }
}
