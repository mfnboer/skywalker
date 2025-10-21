import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyPage {
    required property var skywalker
    required property int purpose // QEnums::ListPurpose
    property listview list
    property bool pickingImage: false
    property string createdAvatarSource
    readonly property int avatarSize: 1000
    readonly property string sideBarTitle: list.isNull() ? qsTr(`New ${(guiSettings.listTypeName(purpose))}`) : qsTr(`Edit ${(guiSettings.listTypeName(purpose))}`)
    readonly property SvgImage sideBarSvg: SvgOutline.list
    readonly property int usableHeight: height - guiSettings.headerMargin - (keyboardHandler.keyboardVisible ? keyboardHandler.keyboardHeight : guiSettings.footerMargin)


    signal closed
    signal listCreated(listview list)
    signal listUpdated(string cid, string name, string description, list<weblink> embeddedLinks, string avatar)

    id: editListPage
    width: parent.width
    clip: true

    Accessible.role: Accessible.Pane

    header: SimpleHeader {
        text: sideBarTitle
        backIsCancel: true
        visible: !root.showSideBar
        onBack: editListPage.cancel()
    }

    footer: Rectangle {
        id: pageFooter
        width: editListPage.width
        height: guiSettings.footerHeight + (keyboardHandler.keyboardVisible ? keyboardHandler.keyboardHeight - guiSettings.footerMargin : 0)
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
            popup.height: Math.min(editListPage.usableHeight, popup.contentHeight)
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
                    editListPage.addEmbeddedLink()
                else if (descriptionField.cursorInEmbeddedLink >= 0)
                    editListPage.updateEmbeddedLink()
            }
        }
    }

    Flickable {
        id: flick
        width: parent.width
        anchors.topMargin: !root.showSideBar ? 0 : guiSettings.headerMargin
        anchors.fill: parent
        clip: true
        contentWidth: pageColumn.width
        contentHeight: pageColumn.y + descriptionRect.y + descriptionField.y + descriptionField.height
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds
        onHeightChanged: descriptionField.ensureVisible(descriptionField.cursorRectangle)

        ColumnLayout {
            id: pageColumn
            x: 10
            y: 10
            width: editListPage.width - 2 * x

            Text {
                Layout.fillWidth: true
                font.bold: true
                color: guiSettings.textColor
                text: qsTr("List Avatar")
            }

            ListAvatar {
                property bool isUpdated: false

                id: avatar
                Layout.preferredWidth: 100
                avatarUrl: list.avatar

                onClicked: {
                    if (pickingImage)
                        return

                    if (Qt.platform.os === "android") {
                        pickingImage = postUtils.pickPhoto()
                    } else {
                        fileDialog.open()
                    }
                }

                Accessible.role: Accessible.Button
                Accessible.name: qsTr("change list avatar")
                Accessible.onPressAction: clicked()

                SvgButton {
                    x: parent.width - width
                    width: 40
                    height: width
                    svg: SvgOutline.close
                    accessibleName: qsTr("delete list avatar")
                    visible: avatar.avatarUrl

                    onClicked: {
                        avatar.setUrl("")
                        dropCreatedAvatar()
                    }
                }

                function setUrl(url) {
                    avatarUrl = url
                    isUpdated = true
                }
            }

            Text {
                Layout.fillWidth: true
                topPadding: 10
                font.bold: true
                color: guiSettings.textColor
                text: qsTr("List Name")
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: nameField.height
                radius: 5
                border.width: 1
                border.color: guiSettings.borderColor
                color: "transparent"

                SkyTextEdit {
                    id: nameField
                    width: parent.width
                    topPadding: 10
                    bottomPadding: 10
                    focus: true
                    initialText: list.name
                    placeholderText: qsTr("List name")
                    singleLine: true
                    maxLength: 64
                    fontSelectorCombo: fontSelector
                }
            }

            Text {
                Layout.fillWidth: true
                topPadding: 10
                font.bold: true
                color: guiSettings.textColor
                text: qsTr("List Description")
            }

            Rectangle {
                id: descriptionRect
                Layout.fillWidth: true
                Layout.preferredHeight: descriptionField.height

                radius: 5
                border.width: 1
                border.color: guiSettings.borderColor
                color: "transparent"

                SkyFormattedTextEdit {
                    id: descriptionField
                    width: parent.width
                    topPadding: 10
                    bottomPadding: 10
                    parentPage: editListPage
                    parentFlick: flick
                    placeholderText: qsTr("List description")
                    initialText: list.description
                    maxLength: 300
                    fontSelectorCombo: fontSelector
                }
            }
        }
    }

    SvgPlainButton {
        id: createListButton
        parent: editListPage.header.visible ? editListPage.header : editListPage
        anchors.right: parent.right
        anchors.rightMargin: 10
        anchors.top: parent.top
        anchors.topMargin: guiSettings.headerMargin
        svg: SvgOutline.check
        iconColor: enabled ? guiSettings.buttonColor : guiSettings.disabledColor
        accessibleName: qsTr("save list")
        enabled: nameField.text.length > 0 && !nameField.maxGraphemeLengthExceeded() && changesMade()

        onClicked: {
            createListButton.enabled = false

            if (list.isNull())
                createList()
            else
                updateList()
        }
    }

    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        running: false
    }

    ImageFileDialog {
        id: fileDialog
        onImageSelected: (fileName) => photoPicked(fileName)
    }

    PostUtils {
        id: postUtils
        skywalker: editListPage.skywalker // qmllint disable missing-type

        onPhotoPicked: (imgSource) => {
            pickingImage = false
            editListPage.photoPicked(imgSource)
        }

        onPhotoPickFailed: (error) => {
            pickingImage = false
            skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
        }

        onPhotoPickCanceled: {
            pickingImage = false
        }
    }

    GraphUtils {
        id: graphUtils
        skywalker: editListPage.skywalker // qmllint disable missing-type

        onCreateListProgress: (msg) => editListPage.createListProgress(msg)
        onCreateListFailed: (error) => editListPage.createListFailed(error)

        // Immediate after creation the list view is not yet available on Bluesky.
        // Therefore we internally create a view.
        onCreateListOk: (uri, cid) => {
            let listView = graphUtils.makeListView(uri, cid, nameField.text, purpose,
                                                   avatar.avatarUrl, skywalker.getUserProfile(),
                                                   descriptionField.text, descriptionField.embeddedLinks)
            editListPage.listCreated(listView)
        }

        onUpdateListProgress: (msg) => editListPage.createListProgress(msg)
        onUpdateListFailed: (error) => editListPage.createListFailed(error)
        onUpdateListOk: (uri, cid) => editListPage.updateListDone(uri, cid)
    }

    VirtualKeyboardHandler {
        id: keyboardHandler
    }

    function createListProgress(msg) {
        busyIndicator.running = true
        skywalker.showStatusMessage(msg, QEnums.STATUS_LEVEL_INFO)
    }

    function createListFailed(error) {
        busyIndicator.running = false
        skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
        createListButton.enabled = true
    }

    function createList() {
        if (descriptionField.checkMisleadingEmbeddedLinks())
            graphUtils.createList(purpose, nameField.text, descriptionField.text, descriptionField.embeddedLinks, avatar.avatarUrl)
    }

    function updateList() {
        if (descriptionField.checkMisleadingEmbeddedLinks())
            graphUtils.updateList(list.uri, nameField.text, descriptionField.text, descriptionField.embeddedLinks, avatar.avatarUrl, avatar.isUpdated)
    }

    function updateListDone(uri, cid) {
        // Erase the created avatar source such that the image will not be dropped
        // from the image provider in Component.onDestruction
        createdAvatarSource = ""
        skywalker.clearStatusMessage()
        editListPage.listUpdated(cid, nameField.text, descriptionField.text, descriptionField.embeddedLinks, avatar.avatarUrl)
    }

    // "file://" or "image://" source
    function photoPicked(source) {
        console.debug("IMAGE:", source)
        let component = guiSettings.createComponent("EditAvatar.qml")
        let page = component.createObject(editListPage, { photoSource: source, relativeRadius: 0.1 })
        page.onClosed.connect(() => { // qmllint disable missing-property
            root.popStack()
            postUtils.dropPhoto(source)
        })
        page.onSelected.connect((rect) => {
            console.debug(rect)
            dropCreatedAvatar()
            createdAvatarSource = postUtils.cutPhotoRect(source, rect, Qt.size(avatarSize, avatarSize))
            avatar.setUrl(createdAvatarSource)
            root.popStack()
            postUtils.dropPhoto(source)
        })
        root.pushStack(page)
    }

    function changesMade() {
        return list.name !== nameField.text ||
                list.description !== descriptionField.text ||
                list.avatar !== avatar.avatarUrl
    }

    function cancel() {
        if (!changesMade()) {
            editListPage.closed()
            return
        }

        guiSettings.askYesNoQuestion(
                    editListPage,
                    qsTr("Do you really want to discard your changes?"),
                    () => editListPage.closed())
    }

    function dropCreatedAvatar() {
        if (createdAvatarSource) {
            postUtils.dropPhoto(createdAvatarSource)
            createdAvatarSource = ""
        }
    }

    function addEmbeddedLink() {
        const webLinkIndex = descriptionField.cursorInWebLink

        if (webLinkIndex < 0)
            return

        console.debug("Web link index:", webLinkIndex, "size:", descriptionField.webLinks.length)
        const webLink = descriptionField.webLinks[webLinkIndex]
        console.debug("Web link:", descriptionField.link)
        let component = guiSettings.createComponent("EditEmbeddedLink.qml")
        let linkPage = component.createObject(editListPage, {
                link: webLink.link,
                canAddLinkCard: false
        })
        linkPage.onAccepted.connect(() => {
                const name = linkPage.getName()
                linkPage.destroy()

                if (name.length > 0)
                    descriptionField.addEmbeddedLink(webLinkIndex, name)

                descriptionField.forceActiveFocus()
        })
        linkPage.onRejected.connect(() => {
                linkPage.destroy()
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
        let linkPage = component.createObject(editListPage, {
                link: link.link,
                name: link.name,
                error: error,
                canAddLinkCard: false
        })
        linkPage.onAccepted.connect(() => {
                const name = linkPage.getName()
                linkPage.destroy()

                if (name.length <= 0)
                    descriptionField.removeEmbeddedLink(linkIndex)
                else
                    descriptionField.updateEmbeddedLink(linkIndex, name)

                descriptionField.forceActiveFocus()
        })
        linkPage.onRejected.connect(() => {
                linkPage.destroy()
                descriptionField.forceActiveFocus()
        })
        linkPage.open()
    }

    Component.onDestruction: {
        dropCreatedAvatar()
    }

    Component.onCompleted: {
        if (!list.isNull())
            descriptionField.setEmbeddedLinks(list.embeddedLinksDescription)

        nameField.forceActiveFocus()
    }
}
