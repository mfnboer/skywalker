import QtQuick
import QtQuick.Controls
import skywalker

AccessibleMenuItem {
    onTriggered: menu.close()

    SvgButton {
        y: 5
        anchors.rightMargin: 10
        anchors.right: parent.right
        width: height
        height: parent.height - 10
        svg: SvgOutline.close
        accessibleName: qsTr("close")
        onClicked: parent.triggered()
    }
}
