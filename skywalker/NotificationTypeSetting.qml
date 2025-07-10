import QtQuick
import QtQuick.Layouts

RowLayout {
    Layout.fillWidth: true
    spacing: -1

    SkyRadioButton {
        Layout.fillWidth: true
        text: qsTr("None")
    }
    SkyRadioButton {
        Layout.fillWidth: true
        text: qsTr("In-app")
    }
    SkyRadioButton {
        Layout.fillWidth: true
        text: qsTr("In-app + Push")
    }
}
