import QtQuick
import QtQuick.Controls.Material
import skywalker

SkyLabel {
    required property real durationSeconds
    readonly property real minOldSeconds: 5 * 24 * 3600
    readonly property bool isOld: durationSeconds >= minOldSeconds
    readonly property real oldDays: (durationSeconds - minOldSeconds) / (24 * 3600)

    labelFontHeight: guiSettings.appFontHeight * 7/8
    labelHeight: labelFontHeight + 2
    text: guiSettings.durationToString(durationSeconds)
    font.pointSize: guiSettings.scaledFont(7/8)
    color: isOld ? guiSettings.textColor : Material.color(Material.Grey)
    backgroundColor: getBackgroundColor()

    MouseArea {
        anchors.fill: parent
        enabled: parent.isOld
        onClicked: root.getSkywalker().showStatusMessage(qsTr("Post is more than 5 days old"), QEnums.STATUS_LEVEL_INFO)
    }

    function getBackgroundColor() {
        if (!isOld)
            return "transparent"

        const factor = Math.min(oldDays * 0.04, 1)

        if (guiSettings.isLightMode)
            return Qt.darker(guiSettings.backgroundColor, 1.01 + factor)
        else
            return Qt.lighter(guiSettings.backgroundColor, 1.6 + factor * 4)
    }
}
