import QtQuick
import skywalker

Rectangle {
    required property SvgImage svg
    required property string text
    property int elide: Text.ElideRight
    property int wrapMode: Text.NoWrap

    signal clicked

    id: rowRect
    height: copyRow.height
    radius: guiSettings.radius
    color: guiSettings.highLightColor(guiSettings.backgroundColor)

    Row {
        id: copyRow
        width: parent.width
        spacing: 5

        AccessibleText {
            id: linkText
            width: parent.width - icon.width - spacing - 10
            anchors.verticalCenter: parent.verticalCenter
            padding: 10
            elide: rowRect.elide
            wrapMode: rowRect.wrapMode
            text: rowRect.text
        }

        SkySvg {
            id: icon
            y: height + 5
            width: guiSettings.appFontHeight + 10
            height: width
            svg: rowRect.svg
            visible: rowRect.enabled
        }

    }

    MouseArea {
        anchors.fill: parent
        enabled: rowRect.enabled
        onClicked: rowRect.clicked()
    }
}
