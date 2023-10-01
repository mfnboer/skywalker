import QtQuick
import QtQuick.Controls

Item {
    // Geometry
    readonly property int footerHeight: 50
    readonly property int footerZLevel: 10
    readonly property int headerHeight: 50
    readonly property int headerZLevel: 10
    readonly property int threadBarWidth: 12 // In 5px units

    // Colors
    readonly property string badgeBorderColor: "white"
    readonly property string badgeColor: "blue"
    readonly property string badgeTextColor: "white"
    readonly property string buttonColor: "blue"
    readonly property string buttonTextColor: "white"
    readonly property string footerColor: "white"
    readonly property string handleColor: "grey"
    readonly property string headerColor: "black"
    readonly property string labelColor: "lightblue"
    readonly property string linkColor: "blue"
    readonly property string postHighLightColor: "aliceblue"
    readonly property string textColor: Material.foreground

    // Font size
    readonly property double labelFontSize: scaledFont(7/8)

    function scaledFont(scaleFactor) {
        return Application.font.pointSize * scaleFactor;
    }

    function durationToString(duration) {
        if (duration < 59.5)
            return Math.round(duration) + qsTr("s", "seconds")

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
