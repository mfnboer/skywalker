import QtQuick
import skywalker

Item {
    required property SvgImage icon
    property string text

    signal clicked()

    id: menuItem
    width: parent.width
    height: itemRow.height

    Accessible.role: Accessible.MenuItem
    Accessible.name: text
    Accessible.description: Accessible.name
    Accessible.onPressAction: menuItem.clicked()

    Row {
        id: itemRow
        width: parent.width
        topPadding: 10
        bottomPadding: 10
        spacing: 20

        SkySvg {
            width: 30
            height: width
            color: guiSettings.textColor
            svg: menuItem.icon
        }
        AccessibleText {
            width: parent.width - 30 - parent.spacing
            anchors.verticalCenter: parent.verticalCenter
            font.pointSize: guiSettings.scaledFont(10/8)
            elide: Text.ElideRight
            text: menuItem.text
        }
    }

    SkyMouseArea {
        anchors.fill: itemRow
        onClicked: menuItem.clicked()
    }

}
