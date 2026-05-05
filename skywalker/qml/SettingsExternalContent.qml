import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

ColumnLayout {
    property var skywalker: root.getSkywalker()
    property var userSettings: skywalker.getUserSettings()

    id: column

    HeaderText {
        Layout.topMargin: 10
        text: qsTr("External content")
    }
    AccessibleCheckBox {
        text: qsTr("Songlink (streaming platforms lookup)")
        checked: userSettings.songlinkEnabled
        onCheckedChanged: userSettings.songlinkEnabled = checked
    }
    AccessibleCheckBox {
        text: qsTr("Use in-app browser to open links")
        checked: userSettings.inAppBrowserEnabled
        onCheckedChanged: userSettings.inAppBrowserEnabled = checked
    }
}
