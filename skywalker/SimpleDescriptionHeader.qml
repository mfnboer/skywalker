import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

Rectangle {
    required property string title
    property string description

    signal closed

    id: header
    width: parent.width
    height: headerColumn.height
    z: guiSettings.headerZLevel
    color: "transparent"

    Column {
        id: headerColumn
        width: parent.width

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
                    svg: SvgOutline.arrowBack
                    accessibleName: qsTr("go back")
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

                    Accessible.role: Accessible.TitleBar
                    Accessible.name: text
                    Accessible.description: Accessible.name
                }
            }
        }
        Rectangle {
            width: parent.width
            height: descriptionText.height
            color: guiSettings.backgroundColor
            visible: description

            Accessible.role: Accessible.StaticText
            Accessible.name: description
            Accessible.description: Accessible.name

            SkyCleanedText {
                id: descriptionText
                width: parent.width
                padding: 10
                wrapMode: Text.Wrap
                elide: Text.ElideRight
                color: guiSettings.textColor
                plainText: description

                Accessible.ignored: true
            }
        }
        Rectangle {
            width: parent.width
            height: 1
            color: guiSettings.separatorColor
            visible: description
        }

    }
}
