import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import skywalker

Item {
    property string avatarUrl
    signal clicked

    id: avatarItem
    height: width

    RoundedFrame {
        id: avatarFrame
        objectToRound: avatarImg
        width: parent.width
        height: parent.height
        radius: parent.width / 2
        visible: avatarItem.avatarUrl && avatarImg.status === Image.Ready

        Image {
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
        color: "blue"
        visible: !avatarFrame.visible

        SvgImage {
            width: parent.width
            height: parent.height
            color: "white"
            svg: svgFilled.unknownAvatar
        }
    }
    MouseArea {
        anchors.fill: parent
        onClicked: {
            console.debug("Avatar clicked");
            avatarItem.clicked()
        }
    }
}
