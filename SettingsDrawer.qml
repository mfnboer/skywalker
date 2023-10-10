import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Drawer {
    property basicprofile user
    readonly property int iconSize: 30
    readonly property real menuFontSize: guiSettings.scaledFont(10/8)

    signal profile()

    padding: 10

    Column {
        id: userColumn
        anchors.top: parent.top
        spacing: 5

        Avatar {
            width: 60
            height: width
            avatarUrl: user.avatarUrl
            onClicked: profile()
        }

        Text {
            wrapMode: Text.Wrap
            elide: Text.ElideRight
            maximumLineCount: 2
            font.bold: true
            text: user.name
        }

        Text {
            elide: Text.ElideRight
            font.pointSize: guiSettings.scaledFont(7/8)
            color: guiSettings.handleColor
            text: user.handle
        }

        Rectangle {
            height: 40
        }

        Row {
            id: profileRow
            spacing: 20

            SvgImage {
                y: height
                width: iconSize
                height: width
                svg: svgOutline.user
            }
            Text {
                anchors.verticalCenter: parent.verticalCenter
                Layout.fillWidth: true
                font.pointSize: menuFontSize
                text: qsTr("Profile")

                MouseArea {
                    anchors.fill: parent
                    onClicked: profile()
                }
            }
        }

        Row {
            spacing: 20

            SvgImage {
                y: height
                width: iconSize
                height: width
                svg: svgOutline.visibility
            }
            Text {
                anchors.verticalCenter: parent.verticalCenter
                Layout.fillWidth: true
                font.pointSize: menuFontSize
                text: qsTr("Moderation")
            }
        }

        Row {
            spacing: 20

            SvgImage {
                y: height
                width: iconSize
                height: width
                svg: svgOutline.settings
            }
            Text {
                anchors.verticalCenter: parent.verticalCenter
                Layout.fillWidth: true
                font.pointSize: menuFontSize
                text: qsTr("Settings")
            }
        }
    }

    GuiSettings {
        id: guiSettings
    }
}
