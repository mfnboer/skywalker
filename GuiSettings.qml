import QtQuick

Item {
    // Geometry
    readonly property int footerHeight: 48
    readonly property int footerZLevel: 10
    readonly property int headerHeight: 48
    readonly property int headerZLevel: 10

    // Colors
    readonly property string footerColor: "white"
    readonly property string headerColor: "black"

    function scaledFont(scaleFactor) {
        return Application.font.pointSize * scaleFactor;
    }
}
