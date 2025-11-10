import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

ColumnLayout {
    property int horizontalPadding: 10
    required property mutedwordentry entry

    id: column

    AccessibleText {
        id: entryText
        Layout.fillWidth: true
        leftPadding: column.horizontalPadding
        rightPadding: column.horizontalPadding
        elide: Text.ElideRight
        wrapMode: Text.Wrap
        font.pointSize: guiSettings.scaledFont(9/8)
        text: entry.value + (entry.isDomain ? " 🔗" : "")
    }
    AccessibleText {
        Layout.fillWidth: true
        leftPadding: column.horizontalPadding
        rightPadding: column.horizontalPadding
        elide: Text.ElideRight
        font.pointSize: guiSettings.scaledFont(7/8)
        color: Material.color(Material.Grey)
        text: getExpiresIndication(entry.expiresAt)
        visible: !isNaN(entry.expiresAt.getTime())
    }

    AccessibleText {
        Layout.fillWidth: true
        leftPadding: column.horizontalPadding
        rightPadding: column.horizontalPadding
        elide: Text.ElideRight
        font.pointSize: guiSettings.scaledFont(7/8)
        color: Material.color(Material.Grey)
        text: qsTr("Exclude users you follow")
        visible: entry.actorTarget === QEnums.ACTOR_TARGET_EXCLUDE_FOLLOWING
    }

    function getExpiresIndication(expiresAt) {
        if (isNaN(expiresAt.getTime()))
            return ""

        const today = new Date()

        if (expiresAt < today)
            return qsTr("Expired")

        return qsTr(`Expires ${guiSettings.expiresIndication(expiresAt)}`)
    }
}
