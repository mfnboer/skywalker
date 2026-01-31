import QtQuick

Row {
    property bool hideFollowing: false
    property bool sync: false

    spacing: 5

    SkyLabel {
        text: qsTr("hide following")
        visible: hideFollowing
    }
    SkyLabel {
        text: qsTr("rewind")
        visible: sync
    }
}
