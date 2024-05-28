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
                accessibleName: qsTr("go back")
                onClicked: page.closed()
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

            HeaderText {
                Layout.columnSpan: 2
                text: qsTr("Account")
            }

            AccessibleText {
                color: guiSettings.textColor
                text: qsTr("Email:")
            }
            RowLayout {
                Layout.fillWidth: true

                AccessibleText {
                    id: mailText
                    Layout.fillWidth: true
                    color: guiSettings.textColor
                    elide: Text.ElideRight
                    text: userPrefs.email
                }
                SvgButton {
                    id: mailConfirmedImg
                    imageMargin: 0
                    implicitWidth: height
                    implicitHeight: mailText.height
                    iconColor: guiSettings.buttonColor
                    Material.background: "transparent"
                    accessibleName: qsTr("E-mail address confirmed")
                    svg: svgOutline.check
                    visible: userPrefs.emailConfirmed
                    onClicked: root.getSkywalker().showStatusMessage(accessibleName, QEnums.STATUS_LEVEL_INFO)
                }
                SvgButton {
                    imageMargin: 0
                    implicitWidth: height
                    implicitHeight: mailText.height
                    iconColor: guiSettings.textColor
                    Material.background: "transparent"
                    accessibleName: qsTr("Two-factor authentication enabled")
                    svg: svgOutline.confirmationCode
                    visible: userPrefs.emailAuthFactor
                    onClicked: root.getSkywalker().showStatusMessage(accessibleName, QEnums.STATUS_LEVEL_INFO)
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
                text: "PDS:"
            }
            AccessibleText {
                Layout.fillWidth: true
                color: guiSettings.textColor
                elide: Text.ElideRight
                text: userPrefs.pds
            }

            AccessibleText {
                id: didLabel
                color: guiSettings.textColor
                text: "DID:"
            }
            RowLayout {
                Layout.fillWidth: true

                AccessibleText {
                    Layout.fillWidth: true
                    color: guiSettings.textColor
                    elide: Text.ElideRight
                    text: userPrefs.did
                }
                SvgButton {
                    imageMargin: 0
                    implicitWidth: height
                    implicitHeight: didLabel.height
                    svg: svgOutline.copy
                    accessibleName: qsTr("copy") + " D I D"
                    iconColor: guiSettings.textColor
                    Material.background: "transparent"
                    onClicked: root.getSkywalker().copyToClipboard(userPrefs.did)
                }
            }

            HeaderText {
                Layout.columnSpan: 2
                Layout.topMargin: 10
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

            HeaderText {
                Layout.topMargin: 10
                Layout.bottomMargin: 10
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

            AccessibleSwitch {
                text: qsTr("Show quotes with blocked post")
                checked: userPrefs.showQuotesWithBlockedPost
                onCheckedChanged: userPrefs.showQuotesWithBlockedPost = checked
            }

            AccessibleSwitch {
                text: qsTr("Rewind to last seen post at startup")
                checked: userPrefs.rewindToLastSeenPost
                onCheckedChanged: userPrefs.rewindToLastSeenPost = checked
            }
        }

        ColumnLayout {
            id: messageColumn
            anchors.top: homeFeedColumn.bottom
            width: parent.width

            HeaderText {
                Layout.topMargin: 10
                text: qsTr("Direct messages")
            }

            AccessibleText {
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                color: guiSettings.textColor
                text: qsTr("Allow new messages from:")
            }

            RadioButton {
                Layout.leftMargin: 10
                padding: 0
                checked: userPrefs.allowIncomingChat === QEnums.ALLOW_INCOMING_CHAT_ALL
                text: qsTr("Everyone")
                onCheckedChanged: {
                    if (checked)
                        userPrefs.allowIncomingChat = QEnums.ALLOW_INCOMING_CHAT_ALL
                }
            }
            RadioButton {
                Layout.leftMargin: 10
                padding: 0
                checked: userPrefs.allowIncomingChat === QEnums.ALLOW_INCOMING_CHAT_FOLLOWING
                text: qsTr("Users I follow")
                onCheckedChanged: {
                    if (checked)
                        userPrefs.allowIncomingChat = QEnums.ALLOW_INCOMING_CHAT_FOLLOWING
                }
            }
            RadioButton {
                Layout.leftMargin: 10
                padding: 0
                checked: userPrefs.allowIncomingChat === QEnums.ALLOW_INCOMING_CHAT_NONE
                text: qsTr("No one")
                onCheckedChanged: {
                    if (checked)
                        userPrefs.allowIncomingChat = QEnums.ALLOW_INCOMING_CHAT_NONE
                }
            }
        }

        ColumnLayout {
            id: languageColumn
            anchors.top: messageColumn.bottom
            width: parent.width

            HeaderText {
                Layout.topMargin: 10
                text: qsTr("Language preferences")
            }
            AccessibleText {
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                color: guiSettings.textColor
                text: qsTr("Which languages do you want to see in feeds (other than home). If no languages are selected than all posts will be shown. Note that the language tags on posts may be wrong or missing.")
            }

            LanguageComboCheckBox {
                Layout.fillWidth: true
                allLanguages: languageUtils.languages
                usedLanguages: languageUtils.usedLanguages
                checkedLangCodes: userPrefs.contentLanguages
                onCheckedLangCodesChanged: userPrefs.contentLanguages = checkedLangCodes
            }

            AccessibleSwitch {
                text: qsTr("Show posts without language tag")
                checked: userPrefs.showUnknownContentLanguage
                onCheckedChanged: userPrefs.showUnknownContentLanguage = checked
            }

            AccessibleSwitch {
                text: qsTr("Show language tags on post")
                checked: userPrefs.showLanguageTags
                onCheckedChanged: userPrefs.showLanguageTags = checked
            }
        }

        ColumnLayout {
            id: appearanceColumn
            anchors.top: languageColumn.bottom
            width: parent.width

            HeaderText {
                Layout.topMargin: 10
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

            HeaderText {
                Layout.topMargin: 10
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

    LanguageUtils {
        id: languageUtils
        skywalker: root.getSkywalker()
    }

    GuiSettings {
        id: guiSettings
    }

    Component.onDestruction: {
        console.debug("Save settings");
        skywalker.saveUserPreferences();
    }
}
