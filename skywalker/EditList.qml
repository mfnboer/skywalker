import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Page {
    required property var skywalker
    required property int purpose // QEnums::ListPurpose
    property listview list
    property bool pickingImage: false
    property string createdAvatarSource
    readonly property int avatarSize: 1000

    signal closed
    signal listCreated(listview list)
    signal listUpdated(string cid, string name, string description, string avatar)

    id: editListPage
    width: parent.width
    clip: true
    Material.background: guiSettings.backgroundColor

    Accessible.role: Accessible.Pane

    header: SimpleHeader {
        text: list.isNull() ? qsTr(`New ${(guiSettings.listTypeName(purpose))}`) : qsTr(`Edit ${(guiSettings.listTypeName(purpose))}`)
        backIsCancel: true
        onBack: editListPage.cancel()

        SvgButton {
            id: createListButton
            anchors.right: parent.right
            anchors.top: parent.top
            svg: svgOutline.check
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
    }

    footer: Rectangle {
        id: pageFooter
        width: editListPage.width
        height: getFooterHeight()
        z: guiSettings.footerZLevel
        color: guiSettings.footerColor
        visible: nameField.activeFocus || descriptionField.activeFocus

        function getFooterHeight() {
            return guiSettings.footerHeight
        }

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
    }

    VirtualKeyboardPageResizer {
        id: virtualKeyboardPageResizer
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
                width: 100
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
                    svg: svgOutline.close
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

                // Normal height acts weird when the user changes the avatar, then it
                // shrinks back to a single line of text. preferredHeight works.
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
                }
            }
        }
    }

    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        running: false
    }

    ImageFileDialog {
        id: fileDialog
        onFileSelected: (fileName) => photoPicked(fileName)
    }

    PostUtils {
        id: postUtils
        skywalker: editListPage.skywalker

        onPhotoPicked: (imgSource) => {
            pickingImage = false
            editListPage.photoPicked(imgSource)
        }

        onPhotoPickFailed: (error) => {
            pickingImage = false
            statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
        }

        onPhotoPickCanceled: {
            pickingImage = false
        }
    }

    GraphUtils {
        id: graphUtils
        skywalker: editListPage.skywalker

        onCreateListProgress: (msg) => editListPage.createListProgress(msg)
        onCreateListFailed: (error) => editListPage.createListFailed(error)

        // Immediate after creation the list view is not yet available on Bluesky.
        // Therefore we internally create a view.
        onCreateListOk: (uri, cid) => {
            let listView = graphUtils.makeListView(uri, cid, nameField.text, purpose,
                                                   avatar.avatarUrl, skywalker.getUserProfile(),
                                                   descriptionField.text)
            editListPage.listCreated(listView)
        }

        onUpdateListProgress: (msg) => editListPage.createListProgress(msg)
        onUpdateListFailed: (error) => editListPage.createListFailed(error)
        onUpdateListOk: (uri, cid) => editListPage.updateListDone(uri, cid)
    }

    GuiSettings {
        id: guiSettings
    }

    function createListProgress(msg) {
        busyIndicator.running = true
        statusPopup.show(msg, QEnums.STATUS_LEVEL_INFO)
    }

    function createListFailed(error) {
        busyIndicator.running = false
        statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
        createListButton.enabled = true
    }

    function createList() {
        graphUtils.createList(purpose, nameField.text, descriptionField.text, avatar.avatarUrl)
    }

    function updateList() {
        graphUtils.updateList(list.uri, nameField.text, descriptionField.text, avatar.avatarUrl, avatar.isUpdated)
    }

    function updateListDone(uri, cid) {
        // Erase the created avatar source such that the image will not be dropped
        // from the image provider in Component.onDestruction
        createdAvatarSource = ""
        statusPopup.close()
        editListPage.listUpdated(cid, nameField.text, descriptionField.text, avatar.avatarUrl)
    }

    // "file://" or "image://" source
    function photoPicked(source) {
        console.debug("IMAGE:", source)
        let component = Qt.createComponent("EditAvatar.qml")
        let page = component.createObject(editListPage, { photoSource: source, relativeRadius: 0.1 })
        page.onClosed.connect(() => {
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

    Component.onDestruction: {
        dropCreatedAvatar()
    }

    Component.onCompleted: {
        // Save the full page height now. Later when the Android keyboard pops up,
        // the page height sometimes changes by itself, but not always...
        virtualKeyboardPageResizer.fullPageHeight = parent.height

        nameField.forceActiveFocus()
    }
}
