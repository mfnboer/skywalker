import QtQuick
import QtQuick.Controls
import skywalker

SvgImage {
    property bool enabled: true

    signal clicked

    id: addImageButton
    width: 34
    height: 34
    color: addImageButton.mustEnable() ? guiSettings.buttonColor : guiSettings.disabledColor
    opacity: 1
    svg: svgOutline.addImage

    Rectangle {
        y: -parent.height
        width: parent.width
        height: parent.height
        color: "transparent"

        Accessible.role: Accessible.Button
        Accessible.name: qsTr("add picture")
        Accessible.onPressAction: if (addImageButton.enabled) addImageButton.clicked()
    }

    MouseArea {
        y: -parent.height
        width: parent.width
        height: parent.height
        enabled: addImageButton.enabled
        onClicked: addImageButton.clicked()
    }
}
