import QtQuick

// Header with a title and a button
Rectangle {
    required property string title
    required property string buttonText

    signal buttonClicked

    id: headerItem
    width: parent.width
    height: guiSettings.headerHeight
    z: guiSettings.headerZLevel
    color: guiSettings.headerColor

    Text {
        anchors.leftMargin: 10
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        font.bold: true
        font.pointSize: guiSettings.scaledFont(10/8)
        color: guiSettings.headerTextColor
        text: headerItem.title
    }

    SkyButton {
        anchors.rightMargin: 10
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        text: headerItem.buttonText
        onClicked: buttonClicked()
    }

    GuiSettings {
        id: guiSettings
    }
}
