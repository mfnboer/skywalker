import QtQuick
import QtQuick.Controls
import skywalker

RadioButton {
    property int horizontalAlignment: Qt.AlignLeft
    property int borderWidth: 1
    readonly property string buttonColor: enabled ? guiSettings.buttonColor : guiSettings.disabledColor

    id: radio
    padding: 0

    indicator: Rectangle { width: 0 }
    display: AbstractButton.TextOnly
    icon.name: ""
    icon.source: ""

    contentItem: Label {
        padding: 5
        background: Rectangle {
            color: radio.checked ? buttonColor : "transparent"
            border.width: radio.borderWidth
            border.color: buttonColor
        }
        horizontalAlignment: radio.horizontalAlignment
        color: checked ? guiSettings.buttonTextColor : buttonColor
        text: radio.text
    }

    Accessible.role: Accessible.RadioButton
    Accessible.name: text
    Accessible.onToggleAction: toggle()

}
