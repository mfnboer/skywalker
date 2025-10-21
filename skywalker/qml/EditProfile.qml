import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyPage {
    required property string authorDid
    property Skywalker skywalker: root.getSkywalker(authorDid)
    required property string authorName
    required property string authorPronouns
    required property string authorDescription
    required property string authorWebsite
    required property string authorAvatar
    required property string authorBanner
    required property bool authorVerified
    property bool pickingImage: false
    property int imageType: QEnums.PHOTO_TYPE_AVATAR
    property string createdAvatarSource
    property string createdBannerSource
    readonly property int avatarSize: 1000
    readonly property int bannerWidth: 3000
    readonly property int bannerHeight: 1000
    readonly property int usableHeight: height - guiSettings.headerMargin - (keyboardHandler.keyboardVisible ? keyboardHandler.keyboardHeight : guiSettings.footerMargin)

    signal closed
    signal profileUpdated(string name, string description, string avatar, string banner, string pronouns, string website)

    id: editProfilePage
    width: parent.width
    height: parent.height
    contentHeight: flick.height

    Accessible.role: Accessible.Pane

    header: SimpleHeader {
        text: qsTr("Edit Profile")
        backIsCancel: true
        onBack: editProfilePage.cancel()

        SvgPlainButton {
            id: updateProfileButton
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.topMargin: guiSettings.headerMargin
            svg: SvgOutline.check
            iconColor: enabled ? guiSettings.buttonColor : guiSettings.disabledColor
            accessibleName: qsTr("save profile")
            enabled: changesMade() && allFieldsValid()

            onClicked: {
                updateProfileButton.enabled = false
                updateProfile()
            }
        }
    }

    footer: Rectangle {
        id: pageFooter
        width: editProfilePage.width
        height: guiSettings.footerHeight + (keyboardHandler.keyboardVisible ? keyboardHandler.keyboardHeight - guiSettings.footerMargin : 0)
        z: guiSettings.footerZLevel
        color: guiSettings.footerColor
        visible: nameField.activeFocus || descriptionField.activeFocus || pronounsField.activeFocus || websiteField.activeFocus

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
            textField: pronounsField
            visible: pronounsField.activeFocus
        }

        TextLengthCounter {
            y: 10
            anchors.rightMargin: 10
            anchors.right: parent.right
            textField: pronounsField
            visible: pronounsField.activeFocus
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
            popup.height: Math.min(editProfilePage.usableHeight, popup.contentHeight)
            focusPolicy: Qt.NoFocus
            visible: nameField.activeFocus || descriptionField.activeFocus || pronounsField.activeFocus
        }
    }

    Flickable {
        id: flick
        anchors.fill: parent
        clip: true
        contentWidth: pageColumn.width
        contentHeight: pageColumn.y + websiteRect.y + websiteField.y + websiteField.height
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds
        onHeightChanged: {
            if (nameField.activeFocus)
                nameField.ensureVisible(nameField.cursorRectangle)
            else if (pronounsField.activeFocus)
                pronounsField.ensureVisible(pronounsField.cursorRectangle)
            else if (descriptionField.activeFocus)
                descriptionField.ensureVisible(descriptionField.cursorRectangle)
            else if (websiteField.activeFocus)
                websiteField.ensureVisible(websiteField.cursorRectangle)
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
                Layout.preferredHeight: parent.width * (bannerHeight / bannerWidth) + avatar.height / 2
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
                        svg: SvgOutline.close
                        accessibleName: qsTr("delete banner")
                        visible: banner.source.toString() !== ""

                        onClicked: {
                            banner.setUrl("")
                            dropCreatedBanner()
                        }
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
                                svg: SvgOutline.close
                                accessibleName: qsTr("delete avatar")
                                visible: avatar.avatarUrl

                                onClicked: {
                                    avatar.setUrl("")
                                    dropCreatedAvatar()
                                }
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
                    parentFlick: flick
                    initialText: authorName
                    placeholderText: qsTr("Your name")
                    singleLine: true
                    maxLength: 64
                    fontSelectorCombo: fontSelector
                }
            }

            AccessibleText {
                Layout.fillWidth: true
                topPadding: 10
                color: guiSettings.errorColor
                wrapMode: Text.Wrap
                text: qsTr("⚠️ Changing your name will invalidate your verified status.")
                visible: authorVerified
            }

            AccessibleText {
                Layout.fillWidth: true
                topPadding: 10
                font.bold: true
                color: guiSettings.textColor
                text: qsTr("Pronouns")
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: pronounsField.height
                radius: 5
                border.width: 1
                border.color: guiSettings.borderColor
                color: "transparent"

                SkyTextEdit {
                    id: pronounsField
                    width: parent.width
                    topPadding: 10
                    bottomPadding: 10
                    focus: true
                    parentFlick: flick
                    initialText: authorPronouns
                    placeholderText: qsTr("Your pronouns")
                    singleLine: true
                    maxLength: 20
                    fontSelectorCombo: fontSelector
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
                    parentPage: editProfilePage
                    parentFlick: flick
                    placeholderText: qsTr("Describe yourself")
                    initialText: authorDescription
                    maxLength: 256
                    enableLinkShortening: false
                    fontSelectorCombo: fontSelector
                }
            }

            AccessibleText {
                Layout.fillWidth: true
                topPadding: 10
                font.bold: true
                color: guiSettings.textColor
                text: qsTr("Website")
            }

            Rectangle {
                id: websiteRect
                Layout.fillWidth: true
                Layout.preferredHeight: websiteField.height
                radius: 5
                border.width: 1
                border.color: guiSettings.borderColor
                color: websiteField.isValid() ? "transparent" : guiSettings.textInputInvalidColor

                SkyTextEdit {
                    id: websiteField
                    width: parent.width
                    topPadding: 10
                    bottomPadding: 10
                    focus: true
                    parentFlick: flick
                    initialText: authorWebsite
                    placeholderText: qsTr("Your website")
                    singleLine: true
                    inputMethodHints: Qt.ImhNoPredictiveText

                    onActiveFocusChanged: {
                        if (activeFocus)
                            websiteField.ensur
                    }

                    function getLink() {
                        const link = text.trim()

                        if (!Boolean(link))
                            return ""
                        else if (linkUtils.hasScheme(link))
                            return link
                        else
                            return "https://" + link
                    }

                    function isValid() {
                        const link = getLink()
                        return !Boolean(link) || linkUtils.isWebLink(link)
                    }
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
        onImageSelected: (fileName) => photoPicked(fileName)
    }

    PostUtils {
        id: postUtils
        skywalker: editProfilePage.skywalker // qmllint disable missing-type

        onPhotoPicked: (imgSource) => {
            pickingImage = false
            editProfilePage.photoPicked(imgSource)
        }

        onPhotoPickFailed: (error) => {
            pickingImage = false
            skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
        }

        onPhotoPickCanceled: {
            pickingImage = false
        }
    }

    ProfileUtils {
        id: profileUtils
        skywalker: editProfilePage.skywalker // qmllint disable missing-type

        onUpdateProfileProgress: (msg) => editProfilePage.updateProfileProgress(msg)
        onUpdateProfileFailed: (error) => editProfilePage.updatProfileFailed(error)
        onUpdateProfileOk: () => editProfilePage.updateProfileDone()
    }

    LinkUtils {
        id: linkUtils
        skywalker: editProfilePage.skywalker
    }

    VirtualKeyboardHandler {
        id: keyboardHandler
    }

    function updateProfileProgress(msg) {
        busyIndicator.running = true
        skywalker.showStatusMessage(msg, QEnums.STATUS_LEVEL_INFO)
    }

    function updatProfileFailed(error) {
        busyIndicator.running = false
        skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
        updateProfileButton.enabled = true
    }

    function updateProfile() {
        profileUtils.updateProfile(authorDid, nameField.text.trim(), descriptionField.text.trim(),
                                   avatar.avatarUrl, avatar.isUpdated,
                                   banner.source, banner.isUpdated,
                                   pronounsField.text.trim(), websiteField.getLink())
    }

    function updateProfileDone() {
        // Erase the created image sources such that the image will not be dropped
        // from the image provider in Component.onDestruction
        createdAvatarSource = ""
        createdBannerSource = ""
        skywalker.clearStatusMessage()
        editProfilePage.profileUpdated(nameField.text.trim(), descriptionField.text.trim(),
                                       avatar.avatarUrl, banner.source,
                                       pronounsField.text.trim(), websiteField.getLink())
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
        let component = guiSettings.createComponent("EditAvatar.qml")
        let page = component.createObject(editProfilePage, { photoSource: source })
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

    // "file://" or "image://" source
    function bannerPhotoPicked(source) {
        let component = guiSettings.createComponent("EditAvatar.qml")

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
        page.onClosed.connect(() => { // qmllint disable missing-property
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
        return authorName !== nameField.text.trim() ||
                authorDescription !== descriptionField.text.trim() ||
                authorAvatar !== avatar.avatarUrl ||
                authorBanner !== banner.source.toString() ||
                authorPronouns !== pronounsField.text.trim() ||
                authorWebsite !== websiteField.getLink()
    }

    function allFieldsValid() {
        return !nameField.maxGraphemeLengthExceeded() &&
                !descriptionField.maxGraphemeLengthExceeded() &&
                !pronounsField.maxGraphemeLengthExceeded() &&
                websiteField.isValid()
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
        nameField.forceActiveFocus()
    }
}
