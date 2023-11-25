import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

// Header with a back button and title text
Rectangle {
    required property string text

    signal back

    id: headerRect
    width: parent.width
    height: guiSettings.headerHeight
    z: guiSettings.headerZLevel
    color: guiSettings.headerColor

    RowLayout
    {
        id: headerRow

        SvgButton {
            id: backButton
            iconColor: guiSettings.headerTextColor
            Material.background: "transparent"
            svg: svgOutline.arrowBack
            onClicked: headerRect.back()
        }
        Text {
            id: headerTexts
            Layout.alignment: Qt.AlignVCenter
            font.bold: true
            font.pointSize: guiSettings.scaledFont(10/8)
            color: guiSettings.headerTextColor
            text: headerRect.text
        }
    }

    GuiSettings {
        id: guiSettings
    }
}
