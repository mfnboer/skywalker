import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Page {
    required property basicprofile author

    signal closed

    id: page
    width: parent.width
    height: parent.height
    topPadding: 10
    bottomPadding: 10

    header: Rectangle {
        width: parent.width
        height: guiSettings.headerHeight
        z: guiSettings.headerZLevel
        color: guiSettings.headerColor

        SkyButton {
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            text: qsTr("Cancel")
            onClicked: page.closed()
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            font.bold: true
            font.pointSize: guiSettings.scaledFont(10/8)
            color: guiSettings.headerTextColor
            text: qsTr("Report Account")
        }

        SkyButton {
            id: reportButton
            anchors.rightMargin: 10
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            text: qsTr("Send");
        }
    }

    RowLayout {
        id: authorRow
        anchors.top: parent.top
        x: 10
        width: parent.width - 10
        spacing: 10

        Avatar {
            width: 40
            height: width
            avatarUrl: author.avatarUrl
        }
        Column {
            Layout.fillWidth: true

            Text {
                elide: Text.ElideRight
                font.bold: true
                color: guiSettings.textColor
                text: author.name
            }
            Text {
                elide: Text.ElideRight
                font.pointSize: guiSettings.scaledFont(7/8)
                color: guiSettings.handleColor
                text: author.handle ? `@${author.handle}` : ""
            }
        }
    }

    ColumnLayout {
        anchors.top: authorRow.bottom
        x: 20
        width: parent.width - 20
        spacing: 10

        RadioButton {
            id: control
            Layout.fillWidth: true

            contentItem: Column {
                anchors.left: control.indicator.right
                anchors.leftMargin: 20

                Text {
                    wrapMode: Text.Wrap
                    font.bold: true
                    color: guiSettings.textColor
                    text: "Title"
                }
                Text {
                    wrapMode: Text.Wrap
                    color: guiSettings.textColor
                    text: "Description"
                }
            }

            Component.onCompleted: control.indicator.x = 0
        }
    }

    GuiSettings {
        id: guiSettings
    }
}
