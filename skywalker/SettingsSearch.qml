import QtQuick
import QtQuick.Layouts

ColumnLayout {
    property var skywalker: root.getSkywalker()
    property var userSettings: skywalker.getUserSettings()

    HeaderText {
        Layout.topMargin: 10
        text: qsTr("Search")
    }

    AccessibleCheckBox {
        text: qsTr("Trending topics")
        checked: userSettings.showTrendingTopics
        onCheckedChanged: userSettings.showTrendingTopics = checked
    }

    AccessibleCheckBox {
        text: qsTr("Suggested accounts")
        checked: userSettings.showSuggestedUsers
        onCheckedChanged: userSettings.showSuggestedUsers = checked
    }
}
