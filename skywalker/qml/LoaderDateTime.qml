import QtQuick
import QtQuick.Controls.Material

Loader {
    required property date postDateTime
    required property bool detailedView
    property bool postVisible: true

    id: dateTimeLoader
    width: parent.width
    active: detailedView && postVisible

    sourceComponent: AccessibleText {
        width: dateTimeLoader.width
        topPadding: 10
        elide: Text.ElideRight
        color: Material.color(Material.Grey)
        text: postDateTime.toLocaleString(Qt.locale(), Locale.ShortFormat)
        font.pointSize: guiSettings.scaledFont(7/8)
    }
}
