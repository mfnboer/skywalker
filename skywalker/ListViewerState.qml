import QtQuick

Row {
    property bool muted
    property string blockedUri

    spacing: 5

    SkyLabel {
        text: qsTr("muted")
        visible: muted
    }
    SkyLabel {
        text: qsTr("blocked")
        visible: blockedUri
    }
}
