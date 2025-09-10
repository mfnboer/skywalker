import QtQuick

Row {
    property bool hideFollowing: false

    spacing: 5

    SkyLabel {
        text: qsTr("hide following")
        visible: hideFollowing
    }
}
