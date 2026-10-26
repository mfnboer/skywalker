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
    required property list<profilelinkview> links
    property bool linkChangesMade: false
    property int imageType: QEnums.PHOTO_TYPE_AVATAR
    property string createdAvatarSource
    property string createdBannerSource
    readonly property int avatarSize: 1000
    readonly property int bannerWidth: 3000
    readonly property int bannerHeight: 1000
    readonly property int usableHeight: height - (keyboardHandler.keyboardVisible ? keyboardHandler.keyboardHeight : 0)

    signal closed
    signal profileUpdated(string name, string description, string avatar, string banner,
                          string pronouns, string website, list<profilelinkview> links)

    id: editProfilePage
    // width: parent.width
    // height: parent.height
    // contentHeight: flick.height
    padding: 10

    Accessible.role: Accessible.Pane

    header: SimpleHeader {
        text: qsTr("Edit Profile")
        backIsCancel: true
        onBack: editProfilePage.cancel()

        SvgPlainButton {
            id: updateProfileButton
            anchors.right: parent.right
            anchors.top: parent.top
            svg: SvgOutline.check
            iconColor: enabled ? guiSettings.buttonColor : guiSettings.disabledColor
            accessibleName: qsTr("save profile")
            enabled: changesMade() && allFieldsValid()

            onClicked: {
                updateProfileButton.enabled = false

                if (profileChangesMade())
                    updateProfile()
                else
                    updateProfileLinks()
            }
        }
    }

    footer: Rectangle {
        id: pageFooter
        width: editProfilePage.width
        height: guiSettings.footerHeight + (keyboardHandler.keyboardVisible ? keyboardHandler.keyboardHeight : 0)
        z: guiSettings.footerZLevel
        color: guiSettings.footerColor
        visible: nameField.activeFocus || descriptionField.activeFocus || pronounsField.activeFocus || websiteField.textInput.activeFocus

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
        contentWidth: parent.width
        //contentHeight: pageColumn.y + websiteField.y + websiteField.height
        contentHeight: contentItem.childrenRect.height
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds

        ColumnLayout {
            id: pageColumn
            width: parent.width

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
                    cache: false

                    Accessible.role: Accessible.Button
                    Accessible.name: qsTr("change banner")
                    Accessible.onPressAction: pickBannerPhoto()

                    SkyMouseArea {
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
                text: qsTr("Name")
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
                text: qsTr("Pronouns")
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: pronounsField.height
                radius: guiSettings.radius
                border.width: pronounsField.activeFocus ? 1 : 0
                border.color: guiSettings.buttonColor
                color: guiSettings.textInputBackgroundColor

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
                text: qsTr("Description")
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
                text: qsTr("Website")
            }

            SkyWebLinkInput {
                id: websiteField
                Layout.fillWidth: true
                parentFlick: flick
                padding: 5
                initialText: authorWebsite
                placeholderText: qsTr("Your website")
            }

            AccessibleText {
                Layout.fillWidth: true
                topPadding: 10
                font.bold: true
                text: qsTr("Link buttons")
            }

            ProfileLinksSorter {
                id: profileLinkSorter
                Layout.fillWidth: true
                Layout.preferredHeight: contentHeight
                links: editProfilePage.links
                onClicked: (index) => editProfilePage.editLink(index)
                onOrderChanged: editProfilePage.checkLinkChangesMade()
            }

            SkyButton {
                Layout.preferredHeight: 40
                text: qsTr("Add link")
                onClicked: editProfilePage.addLink()
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
            editProfilePage.photoPicked(imgSource)
        }

        onPhotoPickFailed: (error) => {
            skywalker.showStatusMessage(error, QEnums.STATUS_LEVEL_ERROR)
        }

        onPhotoPickCanceled: {
            console.debug("Photo pick canceled")
        }
    }

    ProfileUtils {
        id: profileUtils
        skywalker: editProfilePage.skywalker // qmllint disable missing-type

        onUpdateProfileProgress: (msg) => editProfilePage.updateProfileProgress(msg)
        onUpdateProfileFailed: (error) => editProfilePage.updatProfileFailed(error)
        onUpdateProfileOk: () => editProfilePage.updateProfileOk()
        onUpdateProfileLinksOk: (partial) => editProfilePage.updateProfileDone(partial)
        onUpdateProfileLinksFailed: (error) => editProfilePage.updatProfileFailed(error)
    }

    VirtualKeyboardHandler {
        id: keyboardHandler
    }

    function addLink() {
        let component = guiSettings.createComponent("EditProfileLinkButton.qml")
        let dialog = component.createObject(editProfilePage, {})

        dialog.onAccepted.connect(() => {
                const linkView = dialog.getLinkView()

                if (!linkView.isEmpty()) {
                    let links = profileLinkSorter.getLinks()
                    links.push(linkView)
                    profileLinkSorter.links = links
                    checkLinkChangesMade()
                } else {
                    console.warn("Empty link button")
                }

                dialog.close()
            })
        dialog.onRejected.connect(() => dialog.close())

        dialog.open()
    }

    function editLink(linkIndex) {
        const links = profileLinkSorter.getLinks()

        if (linkIndex < 0 || linkIndex >= links.length) {
            console.warn("Invalid link index:", linkIndex, "links:", links)
            return
        }

        const linkView = links[linkIndex]
        let component = guiSettings.createComponent("EditProfileLinkButton.qml")
        let dialog = component.createObject(editProfilePage, { linkView: linkView })

        dialog.onAccepted.connect(() => {
                const linkView = dialog.getLinkView()

                if (!linkView.isEmpty()) {
                    let links = profileLinkSorter.getLinks()
                    links[linkIndex] = linkView
                    profileLinkSorter.links = links
                    checkLinkChangesMade()
                } else {
                    console.warn("Empty link button")
                }

                dialog.close()
            })
        dialog.onDiscarded.connect(() => {
                let links = profileLinkSorter.getLinks()
                links.splice(linkIndex, 1)
                profileLinkSorter.links = links
                checkLinkChangesMade()

                dialog.close()
            })
        dialog.onRejected.connect(() => dialog.close())

        dialog.open()
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

    function updateProfileOk() {
        if (linkChangesMade)
            updateProfileLinks()
        else
            updateProfileDone(false)
    }

    function updateProfileDone(partial) {
        // Erase the created image sources such that the image will not be dropped
        // from the image provider in Component.onDestruction
        createdAvatarSource = ""
        createdBannerSource = ""
        skywalker.clearStatusMessage()

        if (partial)
            skywalker.showStatusMessage(qsTr("Failed to update all link buttons"), QEnums.STATUS_LEVEL_ERROR)
        else
            skywalker.showStatusMessage(qsTr("Profile updated."), QEnums.STATUS_LEVEL_INFO, 2)

        editProfilePage.profileUpdated(nameField.text.trim(), descriptionField.text.trim(),
                                       avatar.avatarUrl, banner.source,
                                       pronounsField.text.trim(), websiteField.getLink(),
                                       profileLinkSorter.getLinks())
        // TODO: links should be modified when update was partial
    }

    function updateProfileLinks() {
        let origIcons = new Set()
        let origLinks = new Map()

        for (const origLink of links) {
            origIcons.add(origLink.icon)

            if (origLink.uri)
                origLinks.set(origLink.uri, origLink)
        }

        let newIcons = []
        let newLinks = profileLinkSorter.getLinks()

        for (const link of newLinks) {
            if (!origIcons.has(link.icon)) {
                console.debug("New icon:", link.icon)
                newIcons.push(link.icon)
            }

            if (origLinks.has(link.uri)) {
                const l = origLinks.get(link.uri)

                if (!l.equals(link))
                    link.setDirty(true)
            }
        }

        profileUtils.updateProfileLinks(authorDid, newLinks, newIcons)
    }

    function pickAvatarPhoto() {
        imageType = QEnums.PHOTO_TYPE_AVATAR
        pickPhoto()
    }

    function pickBannerPhoto() {
        imageType = QEnums.PHOTO_TYPE_BANNER
        pickPhoto()
    }

    function pickPhoto() {
        if (Qt.platform.os === "android") {
            postUtils.pickPhoto(false, 1)
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

    function profileChangesMade() {
        return authorName !== nameField.text.trim() ||
                authorDescription !== descriptionField.text.trim() ||
                authorAvatar !== avatar.avatarUrl ||
                authorBanner !== banner.source.toString() ||
                authorPronouns !== pronounsField.text.trim() ||
                authorWebsite !== websiteField.getLink()
    }

    function checkLinkChangesMade() {
        const newLinks = profileLinkSorter.getLinks()

        if (links.length !== newLinks.length) {
            linkChangesMade = true
            return
        }

        for (let i = 0; i < links.length; ++i) {
            if (!links[i].equals(newLinks[i])) {
                linkChangesMade = true
                return
            }
        }

        linkChangesMade = false
    }

    function changesMade() {
        return profileChangesMade() || linkChangesMade
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
                    qsTr("Do you want to discard your changes?"),
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
        checkLinkChangesMade()
        nameField.forceActiveFocus()
    }
}
