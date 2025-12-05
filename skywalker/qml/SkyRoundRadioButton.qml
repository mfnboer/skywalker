import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

RadioButton {
    id: control
    width: parent.width
    Layout.fillWidth: true
    display: AbstractButton.TextOnly
    icon.name: ""
    icon.source: ""

    contentItem: Text {
        text: control.text
        font.pointSize: guiSettings.scaledFont(1)
        wrapMode: Text.Wrap
        verticalAlignment: Text.AlignVCenter
        anchors.left: control.indicator.right
        anchors.leftMargin: control.spacing
        anchors.right: parent.right
        anchors.rightMargin: control.rightPadding
    }

    Accessible.role: Accessible.Button
    Accessible.name: text
    Accessible.onPressAction: toggle()

    Component.onCompleted: {
        control.indicator.x = control.leftPadding
    }
}
