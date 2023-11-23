import QtQuick
import skywalker

Item {
    required property svgimage icon
    property string text

    signal clicked()

    id: menuItem
    width: itemRow.width
    height: itemRow.height

    Row {
        id: itemRow
        topPadding: 10
        bottomPadding: 10
        spacing: 20

        SvgImage {
            width: 30
            height: width
            color: guiSettings.textColor
            svg: menuItem.icon
        }
        Text {
            anchors.verticalCenter: parent.verticalCenter
            font.pointSize: guiSettings.scaledFont(10/8)
            color: guiSettings.textColor
            text: menuItem.text
        }
    }

    MouseArea {
        anchors.fill: itemRow
        onClicked: menuItem.clicked()
    }

    GuiSettings {
        id: guiSettings
    }
}
