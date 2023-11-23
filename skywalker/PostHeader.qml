import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

GridLayout {
    required property string authorName
    required property string authorHandle
    required property int postIndexedSecondsAgo
    required property int postThreadType

    columns: 2
    rowSpacing: 0

    Text {
        Layout.fillWidth: true
        elide: Text.ElideRight
        text: authorName
        font.bold: true
        color: guiSettings.textColor
    }
    Text {
        text: guiSettings.durationToString(postIndexedSecondsAgo)
        font.pointSize: guiSettings.scaledFont(7/8)
        color: Material.color(Material.Grey)
    }

    Text {
        Layout.columnSpan: 2
        bottomPadding: 5
        elide: Text.ElideRight
        text: "@" + authorHandle
        font.pointSize: guiSettings.scaledFont(7/8)
        color: guiSettings.handleColor
        visible: postThreadType & QEnums.THREAD_ENTRY
    }

    GuiSettings {
        id: guiSettings
    }
}
