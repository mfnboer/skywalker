import QtQuick
import QtQuick.Controls
import skywalker

RadioButton {
    property int horizontalAlignment: Qt.AlignLeft
    property int borderWidth: 1
    readonly property string buttonColor: enabled ? guiSettings.buttonColor : guiSettings.disabledColor
    property string labelColor: "transparent"

    id: radio
    padding: 0

    indicator: Rectangle { width: 0 }
    display: AbstractButton.TextOnly
    icon.name: ""
    icon.source: ""

    contentItem: Label {
        padding: 5
        background: Rectangle {
            color: radio.checked ? buttonColor : labelColor
            border.width: radio.borderWidth
            border.color: buttonColor
        }
        horizontalAlignment: radio.horizontalAlignment
        font.pointSize: guiSettings.scaledFont(1)
        color: checked ? guiSettings.buttonTextColor : buttonColor
        elide: Text.ElideRight
        text: radio.text
    }

    Accessible.role: Accessible.RadioButton
    Accessible.name: text
    Accessible.onToggleAction: toggle()

}
