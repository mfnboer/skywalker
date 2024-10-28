import QtQuick
import QtQuick.Controls
import skywalker

AccessibleMenuItem {
    required property string info

    SvgButton {
        y: 5
        anchors.rightMargin: 10
        anchors.right: parent.right
        width: height
        height: parent.height - 10
        imageMargin: 4
        svg: SvgOutline.info
        accessibleName: qsTr("info")
        onClicked: guiSettings.notice(root, info)
    }

    GuiSettings {
        id: guiSettings
    }
}
