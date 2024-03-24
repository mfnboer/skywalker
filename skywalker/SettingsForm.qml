import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Page {
    required property EditUserPreferences userPrefs

    id: page
    padding: 10

    signal closed()

    Accessible.role: Accessible.Pane

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
                iconColor: guiSettings.headerTextColor
                Material.background: "transparent"
                svg: svgOutline.arrowBack
                onClicked: page.closed()

                Accessible.role: Accessible.Button
                Accessible.name: qsTr("go back")
                Accessible.onPressAction: clicked()
            }
            AccessibleText {
                id: headerTexts
                Layout.alignment: Qt.AlignVCenter
                font.bold: true
                font.pointSize: guiSettings.scaledFont(10/8)
                color: guiSettings.headerTextColor
                text: qsTr("Settings")
            }
        }
    }

    Flickable {
        anchors.fill: parent
        clip: true
        contentWidth: parent.width
        contentHeight: notificationsColumn.y + notificationsColumn.height
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds

        GridLayout {
            id: accountGrid
            width: parent.width
            columns: 2

            AccessibleText {
                Layout.columnSpan: 2
                font.pointSize: guiSettings.scaledFont(9/8)
                font.bold: true
                color: guiSettings.textColor
                text: qsTr("Account")
            }

            AccessibleText {
                color: guiSettings.textColor
                text: qsTr("Email:")
            }
            Rectangle {
                Layout.fillWidth: true
                height: mailText.height
                color: "transparent"

                Accessible.role: Accessible.StaticText
                Accessible.name: userPrefs.email + (userPrefs.emailConfirmed ? qsTr(", confirmed") : "")

                Text {
                    id: mailText
                    color: guiSettings.textColor
                    elide: Text.ElideRight
                    text: userPrefs.email

                    Accessible.ignored: true
                }
                SvgImage {
                    anchors.left: mailText.right
                    anchors.leftMargin: 5
                    height: mailText.height
                    width: height
                    color: guiSettings.buttonColor
                    svg: svgOutline.check
                    visible: userPrefs.emailConfirmed

                    Accessible.ignored: true
                }
            }

            AccessibleText {
                color: guiSettings.textColor
                text: qsTr("Birthday:")
            }
            AccessibleText {
                Layout.fillWidth: true
                color: guiSettings.textColor
                text: userPrefs.birthDate
            }

            AccessibleText {
                color: guiSettings.textColor
                text: qsTr("PDS:")
            }
            AccessibleText {
                Layout.fillWidth: true
                color: guiSettings.textColor
                elide: Text.ElideRight
                text: userPrefs.pds
            }

            AccessibleText {
                Layout.columnSpan: 2
                topPadding: 20
                font.pointSize: guiSettings.scaledFont(9/8)
                font.bold: true
                color: guiSettings.textColor
                text: qsTr("Logged-out visibility")
            }

            Switch {
                id: loggedoutSwitch
                Layout.columnSpan: 2
                Layout.fillWidth: true

                contentItem: Text {
                    text: qsTr("Discourage apps from showing my account to logged-out users");
                    color: guiSettings.textColor
                    wrapMode: Text.Wrap
                    verticalAlignment: Text.AlignVCenter
                    anchors.left: loggedoutSwitch.indicator.right
                    anchors.leftMargin: loggedoutSwitch.spacing
                    anchors.right: parent.right
                    anchors.rightMargin: loggedoutSwitch.rightPadding
                }

                checked: !userPrefs.loggedOutVisibility
                onCheckedChanged: userPrefs.loggedOutVisibility = !checked

                Accessible.role: Accessible.Button
                Accessible.name: contentItem.text
                Accessible.onPressAction: toggle()

                Component.onCompleted: {
                    loggedoutSwitch.indicator.x = loggedoutSwitch.leftPadding
                }
            }
        }

        ColumnLayout {
            id: homeFeedColumn
            anchors.top: accountGrid.bottom
            width: parent.width

            AccessibleText {
                topPadding: 20
                font.pointSize: guiSettings.scaledFont(9/8)
                font.bold: true
                color: guiSettings.textColor
                text: qsTr("Home feed preferences")
            }

            AccessibleSwitch {
                text: qsTr("Show replies")
                checked: !userPrefs.hideReplies
                onCheckedChanged: userPrefs.hideReplies = !checked
            }

            AccessibleSwitch {
                text: qsTr("Replies to followed users only")
                checked: userPrefs.hideRepliesByUnfollowed
                enabled: !userPrefs.hideReplies
                onCheckedChanged: userPrefs.hideRepliesByUnfollowed = checked
            }

            AccessibleSwitch {
                text: qsTr("Show reposts")
                checked: !userPrefs.hideReposts
                onCheckedChanged: userPrefs.hideReposts = !checked
            }

            AccessibleSwitch {
                text: qsTr("Show quote posts")
                checked: !userPrefs.hideQuotePosts
                onCheckedChanged: userPrefs.hideQuotePosts = !checked
            }
        }

        ColumnLayout {
            id: postCompositionColumn
            anchors.top: homeFeedColumn.bottom
            width: parent.width

            AccessibleText {
                topPadding: 20
                font.pointSize: guiSettings.scaledFont(9/8)
                font.bold: true
                color: guiSettings.textColor
                text: qsTr("Post composition")
            }

            AccessibleSwitch {
                text: qsTr("Require ALT text on images")
                checked: userPrefs.requireAltText
                onCheckedChanged: userPrefs.requireAltText = checked
            }
        }

        ColumnLayout {
            id: appearanceColumn
            anchors.top: postCompositionColumn.bottom
            width: parent.width

            AccessibleText {
                topPadding: 20
                font.pointSize: guiSettings.scaledFont(9/8)
                font.bold: true
                color: guiSettings.textColor
                text: qsTr("Appearance")
            }

            RowLayout {
                width: parent.width
                spacing: -1

                SkyRadioButton {
                    Layout.fillWidth: true
                    checked: userPrefs.displayMode === QEnums.DISPLAY_MODE_SYSTEM
                    text: qsTr("System");
                    onCheckedChanged: {
                        if (checked) {
                            userPrefs.displayMode = QEnums.DISPLAY_MODE_SYSTEM
                            root.setDisplayMode(userPrefs.displayMode)
                        }
                    }
                }
                SkyRadioButton {
                    Layout.fillWidth: true
                    checked: userPrefs.displayMode === QEnums.DISPLAY_MODE_LIGHT
                    text: qsTr("Light");
                    onCheckedChanged: {
                        if (checked) {
                            userPrefs.displayMode = QEnums.DISPLAY_MODE_LIGHT
                            root.setDisplayMode(userPrefs.displayMode)
                        }
                    }
                }
                SkyRadioButton {
                    Layout.fillWidth: true
                    checked: userPrefs.displayMode === QEnums.DISPLAY_MODE_DARK
                    text: qsTr("Dark");
                    onCheckedChanged: {
                        if (checked) {
                            userPrefs.displayMode = QEnums.DISPLAY_MODE_DARK
                            root.setDisplayMode(userPrefs.displayMode)
                        }
                    }
                }
            }

            AccessibleSwitch {
                text: qsTr("GIF auto play")
                checked: userPrefs.gifAutoPlay
                onCheckedChanged: userPrefs.gifAutoPlay = checked
            }
        }

        ColumnLayout {
            id: notificationsColumn
            anchors.top: appearanceColumn.bottom
            width: parent.width

            AccessibleText {
                topPadding: 20
                font.pointSize: guiSettings.scaledFont(9/8)
                font.bold: true
                color: guiSettings.textColor
                text: qsTr("Pull notifications")
            }

            AccessibleText {
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                color: guiSettings.textColor
                text: qsTr("Notifications can be enabled/disabled in the app settings of your phone.")
            }

            AccessibleSwitch {
                text: qsTr("WiFi only")
                checked: userPrefs.notificationsWifiOnly
                onCheckedChanged: userPrefs.notificationsWifiOnly = checked
            }
        }
    }

    GuiSettings {
        id: guiSettings
    }

    Component.onDestruction: {
        console.debug("Save settings");
        skywalker.saveUserPreferences();
    }
}
