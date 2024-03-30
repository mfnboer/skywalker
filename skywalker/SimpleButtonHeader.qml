import QtQuick

// Header with a title and a button
Rectangle {
    required property string title
    required property svgimage buttonSvg
    property bool enabled: true

    signal buttonClicked

    id: headerItem
    width: parent.width
    height: guiSettings.headerHeight
    z: guiSettings.headerZLevel
    color: guiSettings.headerColor

    Accessible.role: Accessible.Pane

    Text {
        anchors.leftMargin: 10
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        font.bold: true
        font.pointSize: guiSettings.scaledFont(10/8)
        color: guiSettings.headerTextColor
        text: headerItem.title

        Accessible.role: Accessible.TitleBar
        Accessible.name: text
        Accessible.description: Accessible.name
    }

    SvgButton {
        anchors.rightMargin: 10
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        svg: headerItem.buttonSvg
        enabled: headerItem.enabled
        onClicked: buttonClicked()

        Accessible.role: Accessible.Button
        Accessible.name: qsTr("Ok")
        Accessible.onPressAction: clicked()
    }

    GuiSettings {
        id: guiSettings
    }
}
