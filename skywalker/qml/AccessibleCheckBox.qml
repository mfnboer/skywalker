import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

CheckBox {
    property string textColor: enabled ? guiSettings.textColor : guiSettings.disabledColor

    id: control
    width: parent.width
    Layout.fillWidth: true

    topPadding: 5
    bottomPadding: 5

    contentItem: Text {
        text: control.text
        color: control.textColor
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
