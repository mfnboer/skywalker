import QtQuick

Item {
    width: parent.width
    height: spaceBefore.height + separator.height + spaceAfter.height

    Rectangle {
        id: spaceBefore
        width: parent.width
        height: parent.visible ? 10 : 0
        color: "transparent"
        visible: parent.visible
    }

    Rectangle {
        id: separator
        anchors.top: spaceBefore.bottom
        width: parent.width
        height: parent.visible ? 1 : 0
        color: GuiSettings.separatorColor
        visible: parent.visible
    }

    Rectangle {
        id: spaceAfter
        anchors.top: separator.bottom
        width: parent.width
        height: parent.visible ? 10 : 0
        color: "transparent"
        visible: parent.visible
    }

}
