import QtQuick
import QtQuick.Controls

Button {
    //Material.background: "transparent"
    display: AbstractButton.TextOnly
    icon.name: ""
    icon.source: ""

    background: Rectangle {
        implicitWidth: 100
        implicitHeight: 40
        color: "transparent"
    }
}
