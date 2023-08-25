import QtQuick
import Qt5Compat.GraphicalEffects

Rectangle {
    property string avatarUrl

    id: avatarRect

    Image {
        id: avatarImg
        width: parent.width
        source: avatarUrl
        fillMode: Image.PreserveAspectFit
        visible: false
    }
    Rectangle {
        id: avatarMask
        width: avatarImg.width
        height: avatarImg.height
        radius: width / 2
        visible: false
    }
    OpacityMask {
        anchors.fill: avatarImg
        source: avatarImg
        maskSource: avatarMask
    }
}
