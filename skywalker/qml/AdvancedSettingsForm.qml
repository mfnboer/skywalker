import QtQuick
import QtQuick.Layouts
import skywalker

SkyPage {
    property Skywalker skywalker: root.getSkywalker()
    property var userSettings: skywalker.getUserSettings()
    readonly property string userDid: skywalker.getUserDid()
    readonly property string sideBarTitle: qsTr("Advancedd settings")
    readonly property SvgImage sideBarSvg: SvgOutline.settings

    id: page
    padding: 10

    signal closed()

    Accessible.role: Accessible.Pane

    header: SimpleHeader {
        text: sideBarTitle
        visible: !root.showSideBar
        onBack: cancel()
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

        onHeightChanged: {
            if (appViewField.textInput.activeFocus)
                appViewField.textInput.ensureVisible(appViewField.textInput.cursorRectangle)
            else if (chatField.textInput.activeFocus)
                chatField.textInput.ensureVisible(chatField.textInput.cursorRectangle)
            else if (videoHostField.textInput.activeFocus)
                videoHostField.textInput.ensureVisible(videoHostField.textInput.cursorRectangle)
            else if (videoDidField.textInput.activeFocus)
                videoDidField.textInput.ensureVisible(videoDidField.textInput.cursorRectangle)
        }

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
                initialText: userSettings.getServiceAppView(userDid)
                placeholderText: qsTr("did:web:api.bsky.app#bsky_appview")
                validator: RegularExpressionValidator { regularExpression: /[^ ]*/ }
            }

            SvgButton {
                Layout.preferredWidth: height
                Layout.preferredHeight: appViewField.height
                svg: SvgOutline.close
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
                initialText: userSettings.getServiceChat(userDid)
                validator: RegularExpressionValidator { regularExpression: /[^ ]*/ }
            }

            SvgButton {
                Layout.preferredWidth: height
                Layout.preferredHeight: chatField.height
                svg: SvgOutline.close
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
                initialText: userSettings.getServiceVideoHost(userDid)
                validator: RegularExpressionValidator { regularExpression: /[^ ]*/ }
            }

            SvgButton {
                Layout.preferredWidth: height
                Layout.preferredHeight: videoHostField.height
                svg: SvgOutline.close
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
                initialText: userSettings.getServiceVideoDid(userDid)
                validator: RegularExpressionValidator { regularExpression: /[^ ]*/ }
            }

            SvgButton {
                Layout.preferredWidth: height
                Layout.preferredHeight: videoDidField.height
                svg: SvgOutline.close
                accessibleName: qsTr("reset to default")
                onClicked: videoDidField.text = userSettings.getDefaultServiceVideoDid()
            }
        }
    }

    VirtualKeyboardHandler {
        id: keyboardHandler
    }

    function cancel() {
        closed()
    }

    Component.onCompleted: {
        appViewField.setFocus()
    }
}
