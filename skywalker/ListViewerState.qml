import QtQuick

Row {
    property bool muted: false
    property string blockedUri
    property bool hideFromTimeline: false

    spacing: 5

    SkyLabel {
        text: qsTr("muted")
        visible: muted
    }
    SkyLabel {
        text: qsTr("blocked")
        visible: blockedUri
    }
    SkyLabel {
        text: qsTr("hide from timeline")
        visible: hideFromTimeline
    }
}
