import QtQuick
import QtQuick.Layouts
import skywalker

RowLayout {
    required property var filterablePref

    Layout.fillWidth: true
    spacing: -1

    SkyRadioButton {
        Layout.fillWidth: true
        text: qsTr("None")
        checked: !filterablePref.list
        onCheckedChanged: {
            if (checked)
                filterablePref.list = false
        }
    }
    SkyRadioButton {
        Layout.fillWidth: true
        text: qsTr("In-app")
        checked: filterablePref.list && !filterablePref.push
        onCheckedChanged: {
            if (checked) {
                filterablePref.list = true
                filterablePref.push = false
            }
        }
    }
    SkyRadioButton {
        Layout.fillWidth: true
        text: qsTr("In-app + Push")
        checked: filterablePref.list && filterablePref.push
        onCheckedChanged: {
            if (checked) {
                filterablePref.list = true
                filterablePref.push = true
            }
        }
    }
}
