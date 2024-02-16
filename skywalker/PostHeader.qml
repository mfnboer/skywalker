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

    SkyCleanedText {
        Layout.fillWidth: true
        elide: Text.ElideRight
        plainText: authorName
        font.bold: true
        color: guiSettings.textColor

        Accessible.ignored: true
    }
    Text {
        text: guiSettings.durationToString(postIndexedSecondsAgo)
        font.pointSize: guiSettings.scaledFont(7/8)
        color: Material.color(Material.Grey)

        Accessible.ignored: true
    }

    Text {
        Layout.columnSpan: 2
        bottomPadding: 5
        elide: Text.ElideRight
        text: "@" + authorHandle
        font.pointSize: guiSettings.scaledFont(7/8)
        color: guiSettings.handleColor
        visible: postThreadType & QEnums.THREAD_ENTRY

        Accessible.ignored: true
    }

    GuiSettings {
        id: guiSettings
    }
}
