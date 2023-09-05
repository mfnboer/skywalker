import QtQuick

RoundedFrame {
    property string avatarUrl
    readonly property int status: avatarImg.status

    objectToRound: avatarImg
    radius: width / 2

    Image {
        id: avatarImg
        width: parent.width
        source: avatarUrl
        fillMode: Image.PreserveAspectFit
    }
}
