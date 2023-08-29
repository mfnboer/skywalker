import QtQuick
import QtQuick.Layouts

RowLayout {
    property string authorName
    property int postIndexedSecondsAgo

    Text {
        Layout.fillWidth: true
        elide: Text.ElideRight
        text: authorName
        font.bold: true
    }
    Text {
        text: durationToString(postIndexedSecondsAgo)
        font.pointSize: 8
        color: "grey"
    }

    function durationToString(duration) {
        if (duration < 60)
            return duration + qsTr("s", "seconds")

        duration = duration / 60
        if (duration < 60)
            return Math.round(duration) + qsTr("m", "minutes")

        duration = duration / 60
        if (duration < 24)
            return Math.round(duration) + qsTr("h", "hours")

        duration = duration / 24
        if (duration < 30.4368499)
            return Math.round(duration) + qsTr("d", "days")

        duration = duration / 30.4368499
        if (duration < 36)
            return Math.round(duration) + qsTr("mo", "months")

        duration = duration / 12
        return Math.round(duration) + qsTr("yr", "years")
    }
}
