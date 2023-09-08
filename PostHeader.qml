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
    }
    Text {
        text: durationToString(postIndexedSecondsAgo)
        font.pointSize: root.scaledFont(7/8)
        color: Material.color(Material.Grey)
    }

    Text {
        Layout.columnSpan: 2
        bottomPadding: 5
        elide: Text.ElideRight
        text: authorHandle
        font.pointSize: root.scaledFont(7/8)
        color: Material.color(Material.Grey)
        visible: postThreadType & QEnums.THREAD_ENTRY
    }

    function durationToString(duration) {
        if (duration < 59.5)
            return duration + qsTr("s", "seconds")

        duration = duration / 60
        if (duration < 59.5)
            return Math.round(duration) + qsTr("m", "minutes")

        duration = duration / 60
        if (duration < 23.5)
            return Math.round(duration) + qsTr("h", "hours")

        duration = duration / 24
        if (duration < 30.4368499)
            return Math.round(duration) + qsTr("d", "days")

        duration = duration / 30.4368499
        if (duration < 35.5)
            return Math.round(duration) + qsTr("mo", "months")

        duration = duration / 12
        return Math.round(duration) + qsTr("yr", "years")
    }
}
