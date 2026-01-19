import QtQuick
import skywalker

SkyRadioMenuItem {
    required property int script // QEnums::Script
    property var skywalker: root.getSkywalker()
    property var userSettings: skywalker.getUserSettings()

    text: QEnums.scriptToString(script)
    checked: userSettings.scriptRecognition === script

    onToggled: {
        if (checked)
            userSettings.scriptRecognition = script
    }
}
