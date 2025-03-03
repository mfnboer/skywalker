import QtQuick
import skywalker

AccessibleTabButton {
    signal activated

    implicitWidth: contentItem.width
    Accessible.name: qsTr("press to change tab order")

    contentItem: Rectangle {
        height: parent.height
        width: parent.height + 4
        color: "transparent"

        SkySvg {
            x: 2
            width: parent.height
            height: parent.height
            color: guiSettings.buttonColor
            svg: SvgFilled.settings
        }

        MouseArea {
            anchors.fill: parent
            onClicked: activated()
        }
    }

    onClicked: activated()
}
