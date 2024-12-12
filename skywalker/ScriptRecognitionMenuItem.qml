import QtQuick
import skywalker

AccessibleMenuItem {
    required property int script // QEnums::Script
    property var skywalker: root.getSkywalker()
    property var userSettings: skywalker.getUserSettings()

    text: qEnums.scriptToString(script)
    checkable: true
    checked: userSettings.scriptRecognition == script

    onToggled: {
        if (checked)
            userSettings.scriptRecognition = script
    }

    QEnums {
        id: qEnums
    }
}
