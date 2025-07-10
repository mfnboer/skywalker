import QtQuick
import QtQuick.Layouts

RowLayout {
    Layout.fillWidth: true
    spacing: -1

    SkyRadioButton {
        Layout.fillWidth: true
        text: qsTr("Everyone")
    }
    SkyRadioButton {
        Layout.fillWidth: true
        text: qsTr("Users I follow")
    }
}
