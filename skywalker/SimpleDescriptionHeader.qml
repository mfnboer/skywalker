import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Column {
    required property string title
    property string description

    signal closed

    id: header
    width: parent.width
    z: guiSettings.headerZLevel

    Rectangle {
        width: parent.width
        height: guiSettings.headerHeight
        color: guiSettings.headerColor

        RowLayout {
            id: headerRow
            width: parent.width
            height: guiSettings.headerHeight

            SvgButton {
                id: backButton
                iconColor: guiSettings.headerTextColor
                Material.background: "transparent"
                svg: svgOutline.arrowBack
                onClicked: header.closed()
            }
            Text {
                id: headerTexts
                Layout.alignment: Qt.AlignVCenter
                Layout.fillWidth: true
                leftPadding: 10
                font.bold: true
                font.pointSize: guiSettings.scaledFont(10/8)
                color: guiSettings.headerTextColor
                text: title
            }
        }
    }
    Rectangle {
        width: parent.width
        height: descriptionText.height
        color: guiSettings.backgroundColor
        visible: description

        Text {
            id: descriptionText
            width: parent.width
            padding: 10
            wrapMode: Text.Wrap
            elide: Text.ElideRight
            color: guiSettings.textColor
            text: description
        }
    }
    Rectangle {
        width: parent.width
        height: 1
        color: guiSettings.separatorColor
        visible: description
    }

    GuiSettings {
        id: guiSettings
    }
}
