import QtQuick

// Header with a title and a button
Rectangle {
    required property string title
    required property SvgImage buttonSvg
    property bool enabled: true

    signal buttonClicked

    id: headerItem
    width: parent.width
    height: GuiSettings.headerHeight
    z: GuiSettings.headerZLevel
    color: GuiSettings.headerColor

    Accessible.role: Accessible.Pane

    Text {
        anchors.leftMargin: 10
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        font.bold: true
        font.pointSize: GuiSettings.scaledFont(10/8)
        color: GuiSettings.headerTextColor
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
        accessibleName: qsTr("Ok")
        enabled: headerItem.enabled
        onClicked: buttonClicked()
    }

}
