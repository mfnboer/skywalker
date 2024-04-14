import QtQuick
import QtQuick.Controls

SvgImage {
    required property string accessibleName
    property bool enabled: true

    signal clicked

    id: button
    width: 34
    height: 34
    color: button.enabled ? guiSettings.buttonColor : guiSettings.disabledColor
    opacity: 1

    Rectangle {
        y: -parent.height
        width: parent.width
        height: parent.height
        color: "transparent"

        Accessible.role: Accessible.Button
        Accessible.name: accessibleName
        Accessible.onPressAction: if (button.enabled) button.clicked()
    }

    MouseArea {
        y: -parent.height
        width: parent.width
        height: parent.height
        enabled: button.enabled
        onClicked: button.clicked()
    }

    GuiSettings {
        id: guiSettings
    }
}
