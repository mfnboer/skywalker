import QtQuick

Row {
    property bool muted: false
    property string blockedUri
    property bool hideFromTimeline: false
    property bool hideReplies: false
    property bool hideFollowing: false
    property bool sync: false

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
    SkyLabel {
        text: qsTr("hide replies")
        visible: hideReplies
    }
    SkyLabel {
        text: qsTr("hide following")
        visible: hideFollowing
    }
    SkyLabel {
        text: qsTr("rewind")
        visible: sync
    }
}
