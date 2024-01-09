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

    id: editListPage
    width: parent.width

    header: SimpleHeader {
        text: list.isNull() ? qsTr(`New ${(listTypeName())}`) : qsTr(`Edit ${(listTypeName())}`)
        backIsCancel: true
        onBack: editListPage.closed()

        SvgButton {
            id: createListButton
            anchors.right: parent.right
            anchors.top: parent.top
            svg: svgOutline.check

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
        visible: descriptionField.activeFocus

        function getFooterHeight() {
            return guiSettings.footerHeight
        }

        TextLengthBar {
            textField: descriptionField
        }

        TextLengthCounter {
            y: 10
            anchors.rightMargin: 10
            anchors.right: parent.right
            textField: descriptionField
        }
    }

    Connections {
        target: Qt.inputMethod

        // Resize the footer when the Android virtual keyboard is shown
        function onKeyboardRectangleChanged() {
            if (Qt.inputMethod.keyboardRectangle.y > 0) {
                const keyboardY = Qt.inputMethod.keyboardRectangle.y  / Screen.devicePixelRatio
                pageFooter.height = pageFooter.getFooterHeight() + (parent.height - keyboardY)
            }
            else {
                pageFooter.height = pageFooter.getFooterHeight()
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
        onHeightChanged: ensureVisible(descriptionField.cursorRectangle)

        function ensureVisible(cursor) {
            let cursorY = cursor.y + pageColumn.y + descriptionRect.y + descriptionField.y

            if (contentY >= cursorY)
                contentY = cursorY;
            else if (contentY + height <= cursorY + cursor.height)
                contentY = cursorY + cursor.height - height;
        }

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

            FeedAvatar {
                property bool isUpdated: false

                id: avatar
                width: 180
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

                SvgButton {
                    x: parent.width - width
                    height: width
                    svg: svgOutline.close
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

            SkyTextInput {
                id: nameField
                Layout.fillWidth: true
                focus: true
                initialText: list.name
                placeholderText: qsTr("Name your list")
                maximumLength: 64
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

                radius: 10
                border.width: 1
                border.color: guiSettings.borderColor
                color: "transparent"

                SkyFormattedTextEdit {
                    readonly property int maxLength: 300

                    id: descriptionField
                    width: parent.width
                    topPadding: 10
                    bottomPadding: 10
                    parentPage: editListPage
                    parentFlick: flick
                    placeholderText: qsTr("Describe your list")
                    initialText: list.description
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
        property listview nullList

        id: graphUtils
        skywalker: editListPage.skywalker

        onCreateListProgress: (msg) => editListPage.createListProgress(msg)
        onCreateListFailed: (error) => editListPage.createListFailed(error)

        // I first tried to refresh the list list view to get the new list displayed.
        // That did not work as the new list was apparently not indexed by creator yet.
        // That takes a bit of time.
        // Therefore we get a list view for the new list based on URI instead.
        onCreateListOk: (uri, cid) => graphUtils.getListView(uri)

        onGetListFailed: (error) => editListPage.listCreated(nullList)
        onGetListOk: (listView) => editListPage.listCreated(listView)

        onUpdateListProgress: (msg) => editListPage.createListProgress(msg)
        onUpdateListFailed: (error) => editListPage.createListFailed(error)
        onUpdateListOk: (uri) => graphUtils.getListView(uri)
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

    function listTypeName() {
        switch (purpose) {
        case QEnums.LIST_PURPOSE_MOD:
            return qsTr("Moderation List")
        case QEnums.LIST_PURPOSE_CURATE:
            return qsTr("User List")
        default:
            return qsTr("List")
        }
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
}
