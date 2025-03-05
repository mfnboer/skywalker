import QtQuick
import QtQuick.Controls.Material

SkyLabel {
    required property real durationSeconds
    readonly property bool isOld: durationSeconds >= 30.4368499 * 24 * 3600

    labelFontHeight: guiSettings.appFontHeight * 7/8
    labelHeight: labelFontHeight + 2
    text: guiSettings.durationToString(durationSeconds)
    font.pointSize: guiSettings.scaledFont(7/8)
    color: isOld ? guiSettings.textColor : Material.color(Material.Grey)
    backgroundColor: isOld ? guiSettings.labelColor : "transparent"

    MouseArea {
        anchors.fill: parent
        enabled: parent.isOld
        onClicked: root.getSkywalker().showStatusMessage(qsTr("Post is more than 1 month old"), QEnums.STATUS_LEVEL_INFO)
    }
}
