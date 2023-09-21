import QtQuick

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
    readonly property string headerColor: "black"
    readonly property string linkColor: "blue"

    function scaledFont(scaleFactor) {
        return Application.font.pointSize * scaleFactor;
    }
}
