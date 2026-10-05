import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyPage {
    required property var skywalker
    property starterpackview starterPack
    readonly property string sideBarTitle: starterPack.isNull() ? qsTr("New starter pack") : qsTr("Edit starter pack")
    readonly property SvgImage sideBarSvg: SvgOutline.list
    readonly property int usableHeight: height - (keyboardHandler.keyboardVisible ? keyboardHandler.keyboardHeight : 0)


    signal closed
    signal starterPackCreated(starterpackview starterPack)
    signal starterPackUpdated(string cid, string name, string description, list<namedlink> embeddedLinks)

    id: editPage
    width: parent.width
    clip: true

    Accessible.role: Accessible.Pane

    header: SimpleHeader {
        text: sideBarTitle
        backIsCancel: true
        visible: !root.showSideBar
        onBack: editPage.cancel()
    }

    footer: Rectangle {
        id: pageFooter
        width: editPage.width
        height: guiSettings.footerHeight + (keyboardHandler.keyboardVisible ? keyboardHandler.keyboardHeight : 0)
        z: guiSettings.footerZLevel
        color: guiSettings.footerColor
        visible: nameField.activeFocus || descriptionField.activeFocus

        TextLengthBar {
            textField: nameField
            visible: nameField.activeFocus
        }

        TextLengthCounter {
            y: 10
            anchors.rightMargin: 10
            anchors.right: parent.right
            textField: nameField
            visible: nameField.activeFocus
        }

        TextLengthBar {
            textField: descriptionField
            visible: descriptionField.activeFocus
        }

        TextLengthCounter {
            y: 10
            anchors.rightMargin: 10
            anchors.right: parent.right
            textField: descriptionField
            visible: descriptionField.activeFocus
        }

        FontComboBox {
            id: fontSelector
            x: 10
            y: 10
            popup.height: Math.min(editPage.usableHeight, popup.contentHeight)
            focusPolicy: Qt.NoFocus
        }

        SvgTransparentButton {
            id: linkButton
            anchors.left: fontSelector.right
            anchors.leftMargin: visible ? 8 : 0
            y: 5 + height
            width: visible ? height: 0
            accessibleName: qsTr("embed web link")
            svg: SvgOutline.link
            visible: descriptionField.activeFocus && (descriptionField.cursorInWebLink >= 0 || descriptionField.cursorInEmbeddedLink >= 0)
            onClicked: {
                if (descriptionField.cursorInWebLink >= 0)
                    editPage.addEmbeddedLink()
                else if (descriptionField.cursorInEmbeddedLink >= 0)
                    editPage.updateEmbeddedLink()
            }
        }
    }

    Flickable {
        id: flick
        width: parent.width
        anchors.fill: parent
        clip: true
        contentWidth: pageColumn.width
        contentHeight: pageColumn.y + descriptionRect.y + descriptionField.y + descriptionField.height
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds

        ColumnLayout {
            id: pageColumn
            x: 10
            y: 10
            width: editPage.width - 2 * x

            AccessibleText {
                Layout.fillWidth: true
                topPadding: 10
                font.bold: true
                color: guiSettings.textColor
                text: qsTr("Starter Pack Name")
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: nameField.height
                radius: guiSettings.radius
                border.width: nameField.activeFocus ? 1 : 0
                border.color: guiSettings.buttonColor
                color: guiSettings.textInputBackgroundColor

                SkyTextEdit {
                    id: nameField
                    width: parent.width
                    topPadding: 10
                    bottomPadding: 10
                    focus: true
                    initialText: starterPack.name
                    placeholderText: qsTr("Starter pack name")
                    singleLine: true
                    maxLength: 64
                    fontSelectorCombo: fontSelector
                }
            }

            AccessibleText {
                Layout.fillWidth: true
                topPadding: 10
                font.bold: true
                color: guiSettings.textColor
                text: qsTr("Starter Pack Description")
            }

            Rectangle {
                id: descriptionRect
                Layout.fillWidth: true
                Layout.preferredHeight: descriptionField.height

                radius: guiSettings.radius
                border.width: descriptionField.activeFocus ? 1 : 0
                border.color: guiSettings.buttonColor
                color: guiSettings.textInputBackgroundColor

                SkyFormattedTextEdit {
                    id: descriptionField
                    width: parent.width
                    topPadding: 10
                    bottomPadding: 10
                    parentPage: editPage
                    parentFlick: flick
                    placeholderText: qsTr("Starter pack description")
                    initialText: starterPack.description
                    maxLength: 300
                    fontSelectorCombo: fontSelector
                }
            }
        }
    }

    SvgPlainButton {
        id: createStarterPackButton
        parent: editPage.header.visible ? editPage.header : editPage
        anchors.right: parent.right
        anchors.rightMargin: 10
        anchors.top: parent.top
        svg: SvgOutline.check
        iconColor: enabled ? guiSettings.buttonColor : guiSettings.disabledColor
        accessibleName: qsTr("save starter pack")
        enabled: nameField.text.length > 0 && !nameField.maxGraphemeLengthExceeded() && changesMade()

        onClicked: {
            createStarterPackButton.enabled = false

            if (starterPack.isNull())
                createStarterPack()
            else
                updateStarterPack()
        }
    }

    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        running: false
    }

    GraphUtils {
        id: graphUtils
        skywalker: editPage.skywalker // qmllint disable missing-type

        onCreateStarterPackFailed: (error) => editPage.createStarterPackFailed(error)

        // Immediate after creation the list view is not yet available on Bluesky.
        // Therefore we internally create a view.
        onCreateStarterPackOk: (starterPackUri, starterPackCid, listUri, listCid) => {
                                   let starterPackView = graphUtils.makeStarterPackViewBasic(
                                       starterPackUri, starterPackCid,
                                       nameField.text, skywalker.getUserProfile(),
                                       descriptionField.text, descriptionField.embeddedLinks)
                                   editPage.starterPackCreated(starterPackView)
        }

        onUpdateStarterPackFailed: (error) => editPage.createStarterPackFailed(error)
        onUpdateStarterPackOk: (uri, cid) => editPage.updateStarterPackDone(uri, cid)
    }

    VirtualKeyboardHandler {
        id: keyboardHandler
    }

    function createStarterPackFailed(error) {
        busyIndicator.running = false
        skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
        createStarterPackButton.enabled = true
    }

    function createStarterPack() {
        if (descriptionField.checkMisleadingEmbeddedLinks()) {
            graphUtils.createStarterPack(nameField.text, descriptionField.text, descriptionField.embeddedLinks)
            busyIndicator.running = true
        }
    }

    function updateStarterPack() {
        if (descriptionField.checkMisleadingEmbeddedLinks()) {
            graphUtils.updateStarterPack(starterPack.uri, nameField.text, descriptionField.text, descriptionField.embeddedLinks)
            busyIndicator.running = true
        }
    }

    function updateStarterPackDone(uri, cid) {
        skywalker.clearStatusMessage()
        editPage.starterPackUpdated(cid, nameField.text, descriptionField.text, descriptionField.embeddedLinks)
    }

    function changesMade() {
        return starterPack.name !== nameField.text ||
                starterPack.description !== descriptionField.text
    }

    function cancel() {
        if (!changesMade()) {
            editPage.closed()
            return
        }

        guiSettings.askYesNoQuestion(
                    editPage,
                    qsTr("Do you really want to discard your changes?"),
                    () => editPage.closed())
    }

    function addEmbeddedLink() {
        const webLinkIndex = descriptionField.cursorInWebLink

        if (webLinkIndex < 0)
            return

        console.debug("Web link index:", webLinkIndex, "size:", descriptionField.webLinks.length)
        const webLink = descriptionField.webLinks[webLinkIndex]
        console.debug("Web link:", descriptionField.link)
        let component = guiSettings.createComponent("EditEmbeddedLink.qml")
        let linkPage = component.createObject(editPage, {
                linkType: QEnums.LINK_TYPE_WEB,
                link: webLink.link,
                canAddLinkCard: false
        })
        linkPage.onAccepted.connect(() => {
                const name = linkPage.getName()
                linkPage.close()

                if (name.length > 0)
                    descriptionField.addEmbeddedLink(QEnums.LINK_TYPE_WEB, webLinkIndex, name)

                descriptionField.forceActiveFocus()
        })
        linkPage.onRejected.connect(() => {
                linkPage.close()
                descriptionField.forceActiveFocus()
        })
        linkPage.open()
    }

    function updateEmbeddedLink() {
        const linkIndex = descriptionField.cursorInEmbeddedLink

        if (linkIndex < 0)
            return

        console.debug("Embedded link index:", linkIndex, "size:", descriptionField.embeddedLinks.length)
        const link = descriptionField.embeddedLinks[linkIndex]
        const error = link.hasMisleadingName() ? link.getMisleadingNameError() :
                            (link.isTouchingOtherLink() ? qsTr("Connected to previous link") : "")
        console.debug("Embedded link:", link.link, "name:", link.name, "error:", error)

        let component = guiSettings.createComponent("EditEmbeddedLink.qml")
        let linkPage = component.createObject(editPage, {
                linkType: QEnums.LINK_TYPE_WEB,
                link: link.link,
                name: link.name,
                error: error,
                canAddLinkCard: false
        })
        linkPage.onAccepted.connect(() => {
                const name = linkPage.getName()
                linkPage.close()

                if (name.length <= 0)
                    descriptionField.removeEmbeddedLink(linkIndex)
                else
                    descriptionField.updateEmbeddedLink(linkIndex, name)

                descriptionField.forceActiveFocus()
        })
        linkPage.onRejected.connect(() => {
                linkPage.close()
                descriptionField.forceActiveFocus()
        })
        linkPage.open()
    }

    Component.onCompleted: {
        if (!starterPack.isNull())
            descriptionField.setEmbeddedLinks(starterPack.embeddedLinksDescription)

        nameField.forceActiveFocus()
    }
}
