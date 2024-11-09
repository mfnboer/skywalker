import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyPage {
    required property EditUserPreferences userPrefs
    property var userSettings: root.getSkywalker().getUserSettings()

    id: page
    padding: 10

    signal closed()

    Accessible.role: Accessible.Pane

    header: Rectangle {
        width: parent.width
        height: GuiSettings.headerHeight
        z: GuiSettings.headerZLevel
        color: GuiSettings.headerColor

        RowLayout
        {
            id: headerRow

            SvgButton {
                id: backButton
                iconColor: GuiSettings.headerTextColor
                Material.background: "transparent"
                svg: SvgOutline.arrowBack
                accessibleName: qsTr("go back")
                onClicked: page.closed()
            }
            AccessibleText {
                id: headerTexts
                Layout.alignment: Qt.AlignVCenter
                font.bold: true
                font.pointSize: GuiSettings.scaledFont(10/8)
                color: GuiSettings.headerTextColor
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
                text: qsTr("Email:")
            }
            RowLayout {
                Layout.fillWidth: true

                AccessibleText {
                    id: mailText
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                    text: userPrefs.email
                }
                SvgButton {
                    id: mailConfirmedImg
                    imageMargin: 0
                    implicitWidth: height
                    implicitHeight: mailText.height
                    iconColor: GuiSettings.buttonColor
                    Material.background: "transparent"
                    accessibleName: qsTr("E-mail address confirmed")
                    svg: SvgOutline.check
                    visible: userPrefs.emailConfirmed
                    onClicked: root.getSkywalker().showStatusMessage(accessibleName, QEnums.STATUS_LEVEL_INFO)
                }
                SvgButton {
                    imageMargin: 0
                    implicitWidth: height
                    implicitHeight: mailText.height
                    iconColor: GuiSettings.textColor
                    Material.background: "transparent"
                    accessibleName: qsTr("Two-factor authentication enabled")
                    svg: SvgOutline.confirmationCode
                    visible: userPrefs.emailAuthFactor
                    onClicked: root.getSkywalker().showStatusMessage(accessibleName, QEnums.STATUS_LEVEL_INFO)
                }
            }

            AccessibleText {
                color: GuiSettings.textColor
                text: qsTr("Birthday:")
            }
            AccessibleText {
                Layout.fillWidth: true
                color: GuiSettings.textColor
                text: userPrefs.birthDate
            }

            AccessibleText {
                color: GuiSettings.textColor
                text: "PDS:"
            }
            AccessibleText {
                Layout.fillWidth: true
                color: GuiSettings.textColor
                elide: Text.ElideRight
                text: userPrefs.pds
            }

            AccessibleText {
                id: didLabel
                color: GuiSettings.textColor
                text: "DID:"
            }
            RowLayout {
                Layout.fillWidth: true

                AccessibleText {
                    Layout.fillWidth: true
                    color: GuiSettings.textColor
                    elide: Text.ElideRight
                    text: userPrefs.did
                }
                SvgButton {
                    imageMargin: 0
                    implicitWidth: height
                    implicitHeight: didLabel.height
                    svg: SvgOutline.copy
                    accessibleName: qsTr("copy") + " D I D"
                    iconColor: GuiSettings.textColor
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
                    color: GuiSettings.textColor
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
                text: qsTr("Following feed preferences")
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
                color: GuiSettings.textColor
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
                color: GuiSettings.textColor
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

            GridLayout {
                Layout.fillWidth: true
                columns: 2
                rowSpacing: 5

                AccessibleText {
                    Layout.preferredWidth: 120
                    text: qsTr("Display")
                }

                RowLayout {
                    Layout.fillWidth: true
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

                AccessibleText {
                    Layout.preferredWidth: 120
                    wrapMode: Text.Wrap
                    text: qsTr("Background color")
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 30
                    border.width: 1
                    border.color: GuiSettings.buttonColor
                    color: GuiSettings.backgroundColor

                    MouseArea {
                        anchors.fill: parent
                        onClicked: selectBackgroundColor()
                    }

                    SvgButton {
                        y: -2
                        anchors.right: parent.right
                        width: height
                        height: 34
                        svg: SvgOutline.close
                        accessibleName: qsTr("reset background color to default")
                        onClicked: userSettings.resetBackgroundColor()
                    }
                }

                AccessibleText {
                    Layout.preferredWidth: 120
                    wrapMode: Text.Wrap
                    text: qsTr("Post thread visualisation")
                }

                RowLayout {
                    Layout.fillWidth: true
                    width: parent.width
                    spacing: -1

                    SkyRadioButton {
                        Layout.fillWidth: true
                        checked: userSettings.threadStyle === QEnums.THREAD_STYLE_BAR
                        text: qsTr("Bar");
                        onCheckedChanged: {
                            if (checked)
                                userSettings.threadStyle = QEnums.THREAD_STYLE_BAR
                        }
                    }
                    SkyRadioButton {
                        Layout.fillWidth: true
                        checked: userSettings.threadStyle === QEnums.THREAD_STYLE_LINE
                        text: qsTr("Line");
                        onCheckedChanged: {
                            if (checked)
                                userSettings.threadStyle = QEnums.THREAD_STYLE_LINE
                        }
                    }
                }

                AccessibleText {
                    Layout.preferredWidth: 120
                    wrapMode: Text.Wrap
                    text: qsTr("Post thread color")
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 30
                    border.width: 1
                    border.color: GuiSettings.buttonColor

                    gradient: Gradient {
                        orientation: Gradient.Horizontal
                        GradientStop { position: 0.0; color: GuiSettings.threadStartColor(userSettings.threadColor) }
                        GradientStop { position: 0.7; color: GuiSettings.threadMidColor(userSettings.threadColor) }
                        GradientStop { position: 1.0; color: GuiSettings.threadEndColor(userSettings.threadColor) }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: selectThreadColor()
                    }

                    SvgButton {
                        y: -2
                        anchors.right: parent.right
                        width: height
                        height: 34
                        svg: SvgOutline.close
                        accessibleName: qsTr("reset thread color to default")
                        onClicked: userSettings.resetThreadColor()
                    }
                }
            }

            AccessibleSwitch {
                text: qsTr("Giant emoji")
                checked: userSettings.giantEmojis
                onCheckedChanged: userSettings.giantEmojis = checked
            }

            AccessibleSwitch {
                text: qsTr("GIF auto play")
                checked: userPrefs.gifAutoPlay
                onCheckedChanged: userPrefs.gifAutoPlay = checked
            }

            AccessibleSwitch {
                text: qsTr("Video auto play")
                checked: userSettings.videoAutoPlay
                onCheckedChanged: userSettings.videoAutoPlay = checked
            }

            AccessibleSwitch {
                text: qsTr("Video auto load")
                checked: userSettings.videoAutoLoad
                onCheckedChanged: userSettings.videoAutoLoad = checked
                enabled: !userSettings.videoAutoPlay
            }
            AccessibleText {
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                text: qsTr("With auto loading, the video is automatically loaded (more data usage) so it starts faster when you press play. Otherwise it will load when you press play.")
                enabled: !userSettings.videoAutoPlay
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

    Utils {
        id: utils
    }


    function selectThreadColor() {
        let component = Qt.createComponent("ColorSelector.qml")
        let cs = component.createObject(page)
        cs.selectedColor = userSettings.threadColor
        cs.onRejected.connect(() => cs.destroy())
        cs.onAccepted.connect(() => {
            userSettings.threadColor = cs.selectedColor
            cs.destroy()
        })
        cs.open()
    }

    function selectBackgroundColor() {
        let component = Qt.createComponent("ColorSelector.qml")
        let cs = component.createObject(page)
        cs.selectedColor = userSettings.backgroundColor
        cs.onRejected.connect(() => cs.destroy())
        cs.onAccepted.connect(() => {
            if (checkBackgroundColor(cs.selectedColor))
                userSettings.backgroundColor = cs.selectedColor
            else
                skywalker.showStatusMessage(qsTr("This color would make parts of the app invisible. Pick another color"), QEnums.STATUS_LEVEL_ERROR)
            cs.destroy()
        })
        cs.open()
    }

    function checkBackgroundColor(color) {
        const allowed = GuiSettings.allowedBackgroundColors()

        for (let i = 0; i < allowed.length; ++i)
        {
            if (utils.similarColors(color, allowed[i]))
                return false
        }

        return true
    }

    Component.onDestruction: {
        console.debug("Save settings");
        skywalker.saveUserPreferences();
    }
}
