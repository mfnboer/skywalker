import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Page {
    required property EditUserPreferences userPrefs

    id: page
    padding: 10

    signal closed()

    header: Rectangle {
        width: parent.width
        height: guiSettings.headerHeight
        z: guiSettings.headerZLevel
        color: guiSettings.headerColor

        RowLayout
        {
            id: headerRow

            SvgButton {
                id: backButton
                iconColor: "white"
                Material.background: "transparent"
                svg: svgOutline.arrowBack
                onClicked: page.closed()
            }
            Text {
                id: headerTexts
                Layout.alignment: Qt.AlignVCenter
                font.bold: true
                font.pointSize: guiSettings.scaledFont(10/8)
                color: "white"
                text: qsTr("Settings")
            }
        }
    }

    GridLayout {
        id: accountGrid
        width: parent.width
        columns: 2

        Text {
            Layout.columnSpan: 2
            font.bold: true
            color: guiSettings.textColor
            text: qsTr("Account")
        }

        Text {
            color: guiSettings.textColor
            text: qsTr("Email:")
        }
        Rectangle {
            Layout.fillWidth: true
            height: mailText.height
            color: "transparent"

            Text {
                id: mailText
                color: guiSettings.textColor
                text: userPrefs.email
            }
            SvgImage {
                anchors.left: mailText.right
                anchors.leftMargin: 5
                height: mailText.height
                width: height
                color: guiSettings.buttonColor
                svg: svgOutline.check
                visible: userPrefs.emailConfirmed
            }
        }

        Text {
            color: guiSettings.textColor
            text: qsTr("Birthday:")
        }
        Text {
            Layout.fillWidth: true
            color: guiSettings.textColor
            text: userPrefs.birthDate
        }
    }

    ColumnLayout {
        anchors.top: accountGrid.bottom
        width: parent.width

        Text {
            topPadding: 20
            font.bold: true
            color: guiSettings.textColor
            text: qsTr("Home feed preferences")
        }

        Switch {
            Material.accent: guiSettings.buttonColor
            text: qsTr("Show replies")
            checked: !userPrefs.hideReplies
            onCheckedChanged: userPrefs.hideReplies = !checked
        }

        Switch {
            Material.accent: guiSettings.buttonColor
            text: qsTr("Replies to followed users only")
            checked: userPrefs.hideRepliesByUnfollowed
            enabled: !userPrefs.hideReplies
            onCheckedChanged: userPrefs.hideRepliesByUnfollowed = checked
        }

        Switch {
            Material.accent: guiSettings.buttonColor
            text: qsTr("Show reposts")
            checked: !userPrefs.hideReposts
            onCheckedChanged: userPrefs.hideReposts = !checked
        }

        Switch {
            Material.accent: guiSettings.buttonColor
            text: qsTr("Show quote posts")
            checked: !userPrefs.hideQuotePosts
            onCheckableChanged: userPrefs.hideQuotePosts = !checked
        }
    }

    GuiSettings {
        id: guiSettings
    }

    Component.onDestruction: {
        // TODO: leave page only after saving is done, show busy indicator
        console.debug("Save settings");
        skywalker.saveUserPreferences();
    }
}
