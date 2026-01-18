import QtQuick
import skywalker

LinkCardView {
    signal close
    signal edit

    id: view
    isLiveExternal: true

    LiveLabel {
        x: 5
        y: 10
    }

    SvgButton {
        id: closeButton
        anchors.right: parent.right
        width: 34
        height: width
        svg: SvgOutline.close
        onClicked: view.close()
    }

    SvgButton {
        anchors.right: closeButton.left
        width: 34
        height: width
        svg: SvgOutline.edit
        onClicked: view.edit()
    }
}
