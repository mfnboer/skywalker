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
                iconColor: guiSettings.headerTextColor
                Material.background: "transparent"
                svg: svgOutline.arrowBack
                onClicked: page.closed()
            }
            Text {
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
        contentHeight: appearanceColumn.y + appearanceColumn.height
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds

        GridLayout {
            id: accountGrid
            width: parent.width
            columns: 2

            Text {
                Layout.columnSpan: 2
                font.pointSize: guiSettings.scaledFont(9/8)
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
                    elide: Text.ElideRight
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

            Text {
                color: guiSettings.textColor
                text: qsTr("PDS:")
            }
            Text {
                Layout.fillWidth: true
                color: guiSettings.textColor
                elide: Text.ElideRight
                text: userPrefs.pds
            }

            Text {
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

                Component.onCompleted: {
                    loggedoutSwitch.indicator.x = loggedoutSwitch.leftPadding
                }
            }
        }

        ColumnLayout {
            id: homeFeedColumn
            anchors.top: accountGrid.bottom
            width: parent.width

            Text {
                topPadding: 20
                font.pointSize: guiSettings.scaledFont(9/8)
                font.bold: true
                color: guiSettings.textColor
                text: qsTr("Home feed preferences")
            }

            Switch {
                text: qsTr("Show replies")
                checked: !userPrefs.hideReplies
                onCheckedChanged: userPrefs.hideReplies = !checked
            }

            Switch {
                text: qsTr("Replies to followed users only")
                checked: userPrefs.hideRepliesByUnfollowed
                enabled: !userPrefs.hideReplies
                onCheckedChanged: userPrefs.hideRepliesByUnfollowed = checked
            }

            Switch {
                text: qsTr("Show reposts")
                checked: !userPrefs.hideReposts
                onCheckedChanged: userPrefs.hideReposts = !checked
            }

            Switch {
                text: qsTr("Show quote posts")
                checked: !userPrefs.hideQuotePosts
                onCheckedChanged: userPrefs.hideQuotePosts = !checked
            }
        }

        ColumnLayout {
            id: postCompositionColumn
            anchors.top: homeFeedColumn.bottom
            width: parent.width

            Text {
                topPadding: 20
                font.pointSize: guiSettings.scaledFont(9/8)
                font.bold: true
                color: guiSettings.textColor
                text: qsTr("Post composition")
            }

            Switch {
                text: qsTr("Require ALT text on images")
                checked: userPrefs.requireAltText
                onCheckedChanged: userPrefs.requireAltText = checked
            }
        }

        ColumnLayout {
            id: appearanceColumn
            anchors.top: postCompositionColumn.bottom
            width: parent.width

            Text {
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

            Switch {
                text: qsTr("GIF auto play")
                checked: userPrefs.gifAutoPlay
                onCheckedChanged: userPrefs.gifAutoPlay = checked
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
