import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Page {
    required property var skywalker
    required property string authorDid
    required property string authorName
    required property string authorDescription
    required property string authorAvatar
    required property string authorBanner
    property bool pickingImage: false
    property int imageType: QEnums.PHOTO_TYPE_AVATAR
    property string createdAvatarSource
    property string createdBannerSource
    readonly property int avatarSize: 1000
    readonly property int bannerWidth: 3000
    readonly property int bannerHeight: 1000

    signal closed
    signal profileUpdated(string name, string description, string avatar, string banner)

    id: editProfilePage
    width: parent.width

    Accessible.role: Accessible.Pane

    header: SimpleHeader {
        text: qsTr("Edit Profile")
        backIsCancel: true
        onBack: editProfilePage.cancel()

        SvgButton {
            id: updateProfileButton
            anchors.right: parent.right
            anchors.top: parent.top
            svg: svgOutline.check
            enabled: changesMade() && !nameField.maxGraphemeLengthExceeded()

            onClicked: {
                updateProfileButton.enabled = false
                updateProfile()
            }

            Accessible.role: Accessible.Button
            Accessible.name: qsTr("save profile")
            Accessible.onPressAction: if (enabled) clicked()
        }
    }

    footer: Rectangle {
        id: pageFooter
        width: editProfilePage.width
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
            width: editProfilePage.width - 2 * x

            AccessibleText {
                Layout.fillWidth: true
                font.bold: true
                color: guiSettings.textColor
                text: qsTr("Avatar + Banner")
            }

            Rectangle {
                id: bannerBackground
                Layout.fillWidth: true
                height: parent.width * (bannerHeight / bannerWidth) + avatar.height / 2
                color: guiSettings.bannerDefaultColor

                ImageAutoRetry {
                    property bool isUpdated: false

                    id: banner
                    width: parent.width
                    height: width * (bannerHeight / bannerWidth)
                    source: authorBanner
                    fillMode: Image.PreserveAspectFit

                    Accessible.role: Accessible.Button
                    Accessible.name: qsTr("change banner")
                    Accessible.onPressAction: pickBannerPhoto()

                    MouseArea {
                        anchors.fill: parent
                        onClicked: pickBannerPhoto()
                    }

                    SvgButton {
                        x: parent.width - width
                        width: 40
                        height: width
                        svg: svgOutline.close
                        visible: banner.source.toString() !== ""

                        onClicked: {
                            banner.setUrl("")
                            dropCreatedBanner()
                        }

                        Accessible.role: Accessible.Button
                        Accessible.name: qsTr("delete banner")
                        Accessible.onPressAction: clicked()
                    }

                    function setUrl(url) {
                        source = url
                        banner.isUpdated = true
                    }

                    Rectangle {
                        y: parent.height
                        width: parent.width
                        height: bannerBackground.height - parent.height
                        color: guiSettings.backgroundColor
                    }

                    Rectangle {
                        id: avatarRect
                        x: parent.width - width - 10
                        y: parent.height - height / 2
                        width: 104
                        height: width
                        radius: width / 2
                        color: guiSettings.backgroundColor

                        Avatar {
                            property bool isUpdated: false

                            id: avatar
                            anchors.centerIn: parent
                            width: parent.width - 4
                            avatarUrl: authorAvatar
                            onClicked: pickAvatarPhoto()

                            Accessible.role: Accessible.Button
                            Accessible.name: qsTr("change avatar")
                            Accessible.onPressAction: clicked()

                            SvgButton {
                                x: parent.width - width + 10
                                y: parent.height - height + 10
                                width: 40
                                height: width
                                svg: svgOutline.close
                                visible: avatar.avatarUrl

                                onClicked: {
                                    avatar.setUrl("")
                                    dropCreatedAvatar()
                                }

                                Accessible.role: Accessible.Button
                                Accessible.name: qsTr("delete avatar")
                                Accessible.onPressAction: clicked()
                            }

                            function setUrl(url) {
                                avatarUrl = url
                                avatar.isUpdated = true
                            }
                        }
                    }
                }
            }

            AccessibleText {
                Layout.fillWidth: true
                topPadding: 10
                font.bold: true
                color: guiSettings.textColor
                text: qsTr("Name")
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: nameField.height
                radius: 10
                border.width: 1
                border.color: guiSettings.borderColor
                color: "transparent"

                SkyTextEdit {
                    id: nameField
                    width: parent.width
                    topPadding: 10
                    bottomPadding: 10
                    focus: true
                    initialText: authorName
                    placeholderText: qsTr("Your name")
                    singleLine: true
                    maxGraphemeLength: 64
                }
            }

            AccessibleText {
                Layout.fillWidth: true
                topPadding: 10
                font.bold: true
                color: guiSettings.textColor
                text: qsTr("Description")
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
                    parentPage: editProfilePage
                    parentFlick: flick
                    placeholderText: qsTr("Describe yourself")
                    initialText: authorDescription
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
        skywalker: editProfilePage.skywalker

        onPhotoPicked: (imgSource) => {
            pickingImage = false
            editProfilePage.photoPicked(imgSource)
        }

        onPhotoPickFailed: (error) => {
            pickingImage = false
            statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
        }

        onPhotoPickCanceled: {
            pickingImage = false
        }
    }

    ProfileUtils {
        id: profileUtils
        skywalker: editProfilePage.skywalker

        onUpdateProfileProgress: (msg) => editProfilePage.updateProfileProgress(msg)
        onUpdateProfileFailed: (error) => editProfilePage.updatProfileFailed(error)
        onUpdateProfileOk: () => editProfilePage.updateProfileDone()
    }

    GuiSettings {
        id: guiSettings
    }

    function updateProfileProgress(msg) {
        busyIndicator.running = true
        statusPopup.show(msg, QEnums.STATUS_LEVEL_INFO)
    }

    function updatProfileFailed(error) {
        busyIndicator.running = false
        statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
        updateProfileButton.enabled = true
    }

    function updateProfile() {
        profileUtils.updateProfile(authorDid, nameField.text, descriptionField.text,
                                   avatar.avatarUrl, avatar.isUpdated, banner.source, banner.isUpdated)
    }

    function updateProfileDone() {
        // Erase the created image sources such that the image will not be dropped
        // from the image provider in Component.onDestruction
        createdAvatarSource = ""
        createdBannerSource = ""
        statusPopup.close()
        editProfilePage.profileUpdated(nameField.text, descriptionField.text,
                                       avatar.avatarUrl, banner.source)
    }

    function pickAvatarPhoto() {
        if (pickingImage)
            return

        imageType = QEnums.PHOTO_TYPE_AVATAR
        pickPhoto()
    }

    function pickBannerPhoto() {
        if (pickingImage)
            return

        imageType = QEnums.PHOTO_TYPE_BANNER
        pickPhoto()
    }

    function pickPhoto() {
        if (Qt.platform.os === "android") {
            pickingImage = postUtils.pickPhoto()
        } else {
            fileDialog.open()
        }
    }

    // "file://" or "image://" source
    function avatarPhotoPicked(source) {
        let component = Qt.createComponent("EditAvatar.qml")
        let page = component.createObject(editProfilePage, { photoSource: source })
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

    // "file://" or "image://" source
    function bannerPhotoPicked(source) {
        let component = Qt.createComponent("EditAvatar.qml")

        const pageMargin = 10
        const maskWidth = width - 2 * pageMargin
        const maskHeight = maskWidth * (bannerHeight / bannerWidth)

        let page = component.createObject(editProfilePage, {
            photoSource: source,
            relativeRadius: 0,
            maskWidth: maskWidth,
            maskHeight: maskHeight,
            pageMargin: pageMargin
        })
        page.onClosed.connect(() => {
            root.popStack()
            postUtils.dropPhoto(source)
        })
        page.onSelected.connect((rect) => {
            console.debug(rect)
            dropCreatedBanner()
            createdBannerSource = postUtils.cutPhotoRect(source, rect, Qt.size(bannerWidth, bannerHeight))
            banner.setUrl(createdBannerSource)
            root.popStack()
            postUtils.dropPhoto(source)
        })
        root.pushStack(page)
    }

    // "file://" or "image://" source
    function photoPicked(source) {
        switch (imageType) {
        case QEnums.PHOTO_TYPE_AVATAR:
            avatarPhotoPicked(source)
            break
        case QEnums.PHOTO_TYPE_BANNER:
            bannerPhotoPicked(source)
            break
        }
    }

    function changesMade() {
        return authorName !== nameField.text ||
                authorDescription !== descriptionField.text ||
                authorAvatar !== avatar.avatarUrl ||
                authorBanner !== banner.source.toString()
    }

    function cancel() {
        if (!changesMade()) {
            editProfilePage.closed()
            return
        }

        guiSettings.askYesNoQuestion(
                    editProfilePage,
                    qsTr("Do you really want to discard your changes?"),
                    () => editProfilePage.closed())
    }

    function dropCreatedAvatar() {
        if (createdAvatarSource) {
            postUtils.dropPhoto(createdAvatarSource)
            createdAvatarSource = ""
        }
    }

    function dropCreatedBanner() {
        if (createdBannerSource) {
            postUtils.dropPhoto(createdBannerSource)
            createdBannerSource = ""
        }
    }

    Component.onDestruction: {
        dropCreatedAvatar()
        dropCreatedBanner()
    }

    Component.onCompleted: {
        nameField.setFocus()
    }
}
