import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

// Header with a back button and title text
Rectangle {
    required property string text
    property bool backIsCancel: false

    signal back

    id: headerRect
    width: parent.width
    height: GuiSettings.headerHeight
    z: GuiSettings.headerZLevel
    color: GuiSettings.headerColor

    Accessible.role: Accessible.Pane

    RowLayout
    {
        id: headerRow
        width: parent.width
        Accessible.role: Accessible.Pane

        SvgButton {
            id: backButton
            iconColor: backIsCancel ? GuiSettings.buttonTextColor : GuiSettings.headerTextColor
            Material.background: backIsCancel ? GuiSettings.buttonColor : "transparent"
            svg: backIsCancel ? SvgOutline.cancel : SvgOutline.arrowBack
            accessibleName: backIsCancel ? qsTr("cancel") : qsTr("go back")
            onClicked: headerRect.back()
        }
        Text {
            id: headerTexts
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            font.bold: true
            font.pointSize: GuiSettings.scaledFont(10/8)
            color: GuiSettings.headerTextColor
            elide: Text.ElideRight
            text: headerRect.text

            Accessible.role: Accessible.TitleBar
            Accessible.name: text
            Accessible.description: Accessible.name
        }
    }

}
