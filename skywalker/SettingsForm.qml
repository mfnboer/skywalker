import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts
import skywalker

SkyPage {
    required property EditUserPreferences userPrefs
    property var userSettings: root.getSkywalker().getUserSettings()
    property string userDid: userSettings.getActiveUserDid()

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
                svg: SvgOutline.arrowBack
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
        contentHeight: debugColumn.y + debugColumn.height
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
                    iconColor: guiSettings.buttonColor
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
                    iconColor: guiSettings.textColor
                    Material.background: "transparent"
                    accessibleName: qsTr("Two-factor authentication enabled")
                    svg: SvgOutline.confirmationCode
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
                    svg: SvgOutline.copy
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
                Accessible.name: contentItem.text // qmllint disable missing-property
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
                text: qsTr("Replies in threads from followed users only")
                checked: userSettings.getHideRepliesInThreadFromUnfollowed(userDid)
                enabled: !userPrefs.hideReplies
                onCheckedChanged: userSettings.setHideRepliesInThreadFromUnfollowed(userDid, checked)
            }

            AccessibleSwitch {
                text: qsTr("Replies to followed users only")
                checked: userPrefs.hideRepliesByUnfollowed
                enabled: !userPrefs.hideReplies
                onCheckedChanged: userPrefs.hideRepliesByUnfollowed = checked
            }

            AccessibleSwitch {
                text: qsTr("Assemble post threads")
                checked: userSettings.getAssembleThreads(userDid)
                enabled: !userPrefs.hideReplies
                onCheckedChanged: userSettings.setAssembleThreads(userDid, checked)
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
                checked: userSettings.getShowQuotesWithBlockedPost(userDid)
                onCheckedChanged: userSettings.setShowQuotesWithBlockedPost(userDid, checked)
            }

            AccessibleSwitch {
                text: qsTr("Rewind to last seen post at startup")
                checked: userSettings.getRewindToLastSeenPost(userDid)
                onCheckedChanged: userSettings.setRewindToLastSeenPost(userDid, checked)
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
                checkedLangCodes: userSettings.getContentLanguages(userDid)
                onCheckedLangCodesChanged: userSettings.setContentLanguages(userDid, checkedLangCodes)
            }

            AccessibleSwitch {
                text: qsTr("Show posts without language tag")
                checked: userSettings.getShowUnknownContentLanguage(userDid)
                onCheckedChanged: userSettings.setShowUnknownContentLanguage(userDid, checked)
            }

            AccessibleSwitch {
                text: qsTr("Show language tags on post")
                checked: userSettings.getShowLanguageTags()
                onCheckedChanged: userSettings.setShowLanguageTags(checked)
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
                        checked: userSettings.getDisplayMode() === QEnums.DISPLAY_MODE_SYSTEM
                        text: qsTr("System");
                        onCheckedChanged: {
                            // Avoid setting display mode on initialization of the form.
                            // Sometimes root.setDisplayMode crashes then when it sets the
                            // navigation bar color??
                            if (checked && userSettings.getDisplayMode() !== QEnums.DISPLAY_MODE_SYSTEM) {
                                userSettings.setDisplayMode(QEnums.DISPLAY_MODE_SYSTEM)
                                root.setDisplayMode(userSettings.getDisplayMode())
                            }
                        }
                    }
                    SkyRadioButton {
                        Layout.fillWidth: true
                        checked: userSettings.getDisplayMode() === QEnums.DISPLAY_MODE_LIGHT
                        text: qsTr("Light");
                        onCheckedChanged: {
                            if (checked && userSettings.getDisplayMode() !== QEnums.DISPLAY_MODE_LIGHT) {
                                userSettings.setDisplayMode(QEnums.DISPLAY_MODE_LIGHT)
                                root.setDisplayMode(userSettings.getDisplayMode())
                            }
                        }
                    }
                    SkyRadioButton {
                        Layout.fillWidth: true
                        checked: userSettings.getDisplayMode() === QEnums.DISPLAY_MODE_DARK
                        text: qsTr("Dark");
                        onCheckedChanged: {
                            if (checked && userSettings.getDisplayMode() !== QEnums.DISPLAY_MODE_DARK) {
                                userSettings.setDisplayMode(QEnums.DISPLAY_MODE_DARK)
                                root.setDisplayMode(userSettings.getDisplayMode())
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
                    Layout.preferredHeight: 30
                    border.width: 1
                    border.color: guiSettings.buttonColor
                    color: guiSettings.backgroundColor

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

                Rectangle {
                    Layout.preferredWidth: 120
                    Layout.preferredHeight: accentColorLabel.height
                    color: "transparent"

                    AccessibleText {
                        id: accentColorLabel
                        width: parent.width - accentColorInfoButton.width
                        wrapMode: Text.Wrap
                        text: qsTr("Accent color")
                    }
                    SvgButton {
                        id: accentColorInfoButton
                        anchors.right: parent.right
                        width: 34
                        height: width
                        imageMargin: 4
                        svg: SvgOutline.info
                        accessibleName: qsTr("info")
                        onClicked: guiSettings.notice(root, qsTr("Accent color is used for buttons, default avatar and counter badges"))
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    border.width: 1
                    border.color: guiSettings.accentColor
                    color: guiSettings.accentColor

                    MouseArea {
                        anchors.fill: parent
                        onClicked: selectAccentColor()
                    }

                    SvgButton {
                        y: -2
                        anchors.right: parent.right
                        width: height
                        height: 34
                        svg: SvgOutline.close
                        accessibleName: qsTr("reset accent color to default")
                        onClicked: {
                            userSettings.resetAccentColor()
                            root.Material.accent = userSettings.accentColor
                        }
                    }
                }

                AccessibleText {
                    Layout.preferredWidth: 120
                    wrapMode: Text.Wrap
                    text: qsTr("Link color")
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    border.width: 1
                    border.color: guiSettings.buttonColor
                    color: guiSettings.linkColor

                    MouseArea {
                        anchors.fill: parent
                        onClicked: selectLinkColor()
                    }

                    SvgButton {
                        y: -2
                        anchors.right: parent.right
                        width: height
                        height: 34
                        svg: SvgOutline.close
                        accessibleName: qsTr("reset link color to default")
                        onClicked: userSettings.resetLinkColor()
                    }
                }

                AccessibleText {
                    Layout.preferredWidth: 120
                    wrapMode: Text.Wrap
                    text: qsTr("Post thread visualisation")
                }

                RowLayout {
                    Layout.fillWidth: true
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
                    Layout.preferredHeight: 30
                    border.width: 1
                    border.color: guiSettings.buttonColor

                    gradient: Gradient {
                        orientation: Gradient.Horizontal
                        GradientStop { position: 0.0; color: guiSettings.threadStartColor(userSettings.threadColor) }
                        GradientStop { position: 0.7; color: guiSettings.threadMidColor(userSettings.threadColor) }
                        GradientStop { position: 1.0; color: guiSettings.threadEndColor(userSettings.threadColor) }
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
                text: qsTr("Floating navigation buttons")
                checked: userSettings.floatingNavigationButtons
                onCheckedChanged: userSettings.floatingNavigationButtons = checked
            }

            AccessibleSwitch {
                text: qsTr("Giant emoji")
                checked: userSettings.giantEmojis
                onCheckedChanged: userSettings.giantEmojis = checked
            }

            AccessibleSwitch {
                text: qsTr("GIF auto play")
                checked: userSettings.getGifAutoPlay()
                onCheckedChanged: userSettings.setGifAutoPlay(checked)
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
                checked: userSettings.getNotificationsWifiOnly()
                onCheckedChanged: userSettings.setNotificationsWifiOnly(checked)
            }
        }

        ColumnLayout {
            id: debugColumn
            anchors.top: notificationsColumn.bottom
            width: parent.width

            HeaderText {
                Layout.topMargin: 10
                text: qsTr("Debug")
            }

            AccessibleSwitch {
                text: qsTr("Disable emoji font")
                checked: userSettings.getDebugDisableEmojiFont()
                onCheckedChanged: userSettings.setDebugDisableEmojiFont(checked)
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
            if (checkColor(cs.selectedColor, guiSettings.forbiddenBackgroundColors()))
                userSettings.backgroundColor = cs.selectedColor
            else
                skywalker.showStatusMessage(qsTr("This color would make parts of the app invisible. Pick another color"), QEnums.STATUS_LEVEL_ERROR)
            cs.destroy()
        })
        cs.open()
    }

    function selectAccentColor() {
        let component = Qt.createComponent("ColorSelector.qml")
        let cs = component.createObject(page)
        cs.selectedColor = userSettings.accentColor
        cs.onRejected.connect(() => cs.destroy())
        cs.onAccepted.connect(() => {
            if (checkColor(cs.selectedColor, guiSettings.forbiddenAccentColors())) {
                userSettings.accentColor = cs.selectedColor
                root.Material.accent = cs.selectedColor
            }
            else {
                skywalker.showStatusMessage(qsTr("This color would make parts of the app invisible. Pick another color"), QEnums.STATUS_LEVEL_ERROR)
            }
            cs.destroy()
        })
        cs.open()
    }

    function selectLinkColor() {
        let component = Qt.createComponent("ColorSelector.qml")
        let cs = component.createObject(page)
        cs.selectedColor = userSettings.accentColor
        cs.onRejected.connect(() => cs.destroy())
        cs.onAccepted.connect(() => {
            if (checkColor(cs.selectedColor, guiSettings.forbiddenLinkColors()))
                userSettings.linkColor = cs.selectedColor
            else
                skywalker.showStatusMessage(qsTr("This color would make some links invisible. Pick another color"), QEnums.STATUS_LEVEL_ERROR)
            cs.destroy()
        })
        cs.open()
    }

    function checkColor(color, forbiddenColors) {
        for (let i = 0; i < forbiddenColors.length; ++i)
        {
            if (utils.similarColors(color, forbiddenColors[i]))
                return false
        }

        return true
    }

    Component.onDestruction: {
        console.debug("Save settings");
        skywalker.saveUserPreferences();
    }
}
