import QtQuick

RoundedFrame {
    property string avatarUrl

    objectToRound: avatarImg
    radius: width / 2

    Image {
        id: avatarImg
        width: parent.width
        source: avatarUrl
        fillMode: Image.PreserveAspectFit
    }
}
