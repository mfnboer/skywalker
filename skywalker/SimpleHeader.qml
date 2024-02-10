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
    height: guiSettings.headerHeight
    z: guiSettings.headerZLevel
    color: guiSettings.headerColor

    Accessible.role: Accessible.Pane

    RowLayout
    {
        id: headerRow
        width: parent.width
        Accessible.role: Accessible.Pane

        SvgButton {
            id: backButton
            iconColor: backIsCancel ? guiSettings.buttonTextColor : guiSettings.headerTextColor
            Material.background: backIsCancel ? guiSettings.buttonColor : "transparent"
            svg: backIsCancel ? svgOutline.cancel : svgOutline.arrowBack
            onClicked: headerRect.back()

            Accessible.role: Accessible.Button
            Accessible.name: backIsCancel ? qsTr("cancel") : qsTr("go back")
            Accessible.description: backIsCancel ? "" : qsTr("Go back to the previous page.")
            Accessible.onPressAction: clicked()
        }
        Text {
            id: headerTexts
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            font.bold: true
            font.pointSize: guiSettings.scaledFont(10/8)
            color: guiSettings.headerTextColor
            elide: Text.ElideRight
            text: headerRect.text

            Accessible.role: Accessible.TitleBar
            Accessible.name: text
            Accessible.description: Accessible.name
        }
    }

    GuiSettings {
        id: guiSettings
    }
}
