import QtQuick
import QtQuick.Layouts
import skywalker

SkyPage {
    property bool newUser: false
    property Skywalker skywalker: root.getSkywalker()
    property var userSettings: skywalker.getUserSettings()
    readonly property string userDid: skywalker.getUserDid()
    readonly property string sideBarTitle: qsTr("Advanced settings")
    readonly property SvgImage sideBarSvg: SvgOutline.settings

    id: page
    padding: 10

    signal closed()
    signal settings(string serviceAppView, string serviceChat, string serviceVideoHost, string serviceVideoHost)

    Accessible.role: Accessible.Pane

    header: SimpleHeader {
        text: sideBarTitle
        visible: !root.showSideBar
        onBack: cancel()

        SvgPlainButton {
            id: saveButton
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.topMargin: guiSettings.headerMargin
            svg: SvgOutline.check
            iconColor: enabled ? guiSettings.buttonColor : guiSettings.disabledColor
            accessibleName: qsTr("save settings")
            enabled: changesMade() && allFieldsValid()

            onClicked: {
                saveSettings()
                closed()
            }
        }
    }

    footer: DeadFooterMargin {
        height: keyboardHandler.keyboardVisible ? keyboardHandler.keyboardHeight : guiSettings.footerMargin
    }

    Flickable {
        id: flick
        anchors.topMargin: !root.showSideBar ? 0 : guiSettings.headerMargin
        anchors.fill: parent
        clip: true
        contentWidth: parent.width
        contentHeight: contentItem.childrenRect.height
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds

        GridLayout {
            columns: 2
            width: parent.width

            HeaderText {
                Layout.columnSpan: 2
                Layout.fillWidth: true
                text: qsTr("ATProto")
            }

            AccessibleText {
                Layout.columnSpan: 2
                Layout.fillWidth: true
                topPadding: 10
                font.bold: true
                text: "AppView service"
            }

            SkyTextInput {
                id: appViewField
                Layout.fillWidth: true
                parentFlick: flick
                inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
                initialText: newUser ? userSettings.getDefaultServiceAppView() : userSettings.getServiceAppView(userDid)
                placeholderText: qsTr("did:web:api.bsky.app#bsky_appview")
                validator: RegularExpressionValidator { regularExpression: /[^ ]*/ }
                valid: !displayText || linkUtils.isValidService(displayText)
            }

            SvgPlainButton {
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredWidth: height
                Layout.preferredHeight: appViewField.height - 10
                imageMargin: 0
                svg: SvgOutline.undo
                accessibleName: qsTr("reset to default")
                onClicked: appViewField.text = userSettings.getDefaultServiceAppView()
            }

            AccessibleText {
                Layout.columnSpan: 2
                Layout.fillWidth: true
                topPadding: 10
                font.bold: true
                text: "Chat service"
            }

            SkyTextInput {
                id: chatField
                Layout.fillWidth: true
                parentFlick: flick
                inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
                initialText: newUser ? userSettings.getDefaultServiceChat() : userSettings.getServiceChat(userDid)
                validator: RegularExpressionValidator { regularExpression: /[^ ]*/ }
                valid: !displayText || linkUtils.isValidService(displayText)
            }

            SvgPlainButton {
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredWidth: height
                Layout.preferredHeight: chatField.height - 10
                imageMargin: 0
                svg: SvgOutline.undo
                accessibleName: qsTr("reset to default")
                onClicked: chatField.text = userSettings.getDefaultServiceChat()
            }

            AccessibleText {
                Layout.columnSpan: 2
                Layout.fillWidth: true
                topPadding: 10
                font.bold: true
                text: "Video upload host"
            }

            SkyTextInput {
                id: videoHostField
                Layout.fillWidth: true
                parentFlick: flick
                inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
                initialText: newUser ? userSettings.getDefaultServiceVideoHost() : userSettings.getServiceVideoHost(userDid)
                validator: RegularExpressionValidator { regularExpression: /[^ ]*/ }
                valid: isValid()

                function getLink() {
                    return linkUtils.getLinkWithScheme(displayText)
                }

                function isValid() {
                    const link = getLink()
                    return linkUtils.isWebLink(link)
                }
            }

            SvgPlainButton {
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredWidth: height
                Layout.preferredHeight: videoHostField.height - 10
                imageMargin: 0
                svg: SvgOutline.undo
                accessibleName: qsTr("reset to default")
                onClicked: videoHostField.text = userSettings.getDefaultServiceVideoHost()
            }

            AccessibleText {
                Layout.columnSpan: 2
                Layout.fillWidth: true
                topPadding: 10
                font.bold: true
                text: "Video service DID"
            }

            SkyTextInput {
                id: videoDidField
                Layout.fillWidth: true
                parentFlick: flick
                inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText
                initialText: newUser ? userSettings.getDefaultServiceVideoDid() : userSettings.getServiceVideoDid(userDid)
                validator: RegularExpressionValidator { regularExpression: /[^ ]*/ }
                valid: linkUtils.isValidDid(displayText)
            }

            SvgPlainButton {
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredWidth: height
                Layout.preferredHeight: videoDidField.height - 10
                imageMargin: 0
                svg: SvgOutline.undo
                accessibleName: qsTr("reset to default")
                onClicked: videoDidField.text = userSettings.getDefaultServiceVideoDid()
            }
        }
    }

    LinkUtils {
        id: linkUtils
        skywalker: page.skywalker
    }

    VirtualKeyboardHandler {
        id: keyboardHandler
    }

    function changesMade() {
        if (newUser) {
            return appViewField.displayText !== userSettings.getDefaultServiceAppView() ||
                    chatField.displayText !== userSettings.getDefaultServiceChat() ||
                    videoHostField.displayText !== userSettings.getDefaultServiceVideoHost() ||
                    videoDidField.displayText !== userSettings.getDefaultServiceVideoDid()
        }

        return appViewField.displayText !== userSettings.getServiceAppView(userDid) ||
                chatField.displayText !== userSettings.getServiceChat(userDid) ||
                videoHostField.displayText !== userSettings.getServiceVideoHost(userDid) ||
                videoDidField.displayText !== userSettings.getServiceVideoDid(userDid)
    }

    function allFieldsValid() {
        return appViewField.valid &&
                chatField.valid &&
                videoHostField.valid &&
                videoDidField.valid
    }

    function saveSettings() {
        if (newUser) {
            settings(appViewField.displayText, chatField.displayText, videoHostField.displayText, videoDidField.displayText)
            return
        }

        userSettings.setServiceAppView(userDid, appViewField.displayText)
        userSettings.setServiceChat(userDid, chatField.displayText)
        userSettings.setServiceVideoHost(userDid, videoHostField.displayText)
        userSettings.setServiceVideoDid(userDid, videoDidField.displayText)
    }

    function cancel() {
        if (!changesMade()) {
            page.closed()
            return
        }

        guiSettings.askYesNoQuestion(
                    page,
                    qsTr("Do you want to discard your changes?"),
                    () => page.closed())
    }

    Component.onCompleted: {
        appViewField.setFocus()
    }
}
