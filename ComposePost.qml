import QtCore
import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import QtQuick.Window 2.2
import skywalker

Page {
    required property var skywalker
    property string initialText
    property string initialImage

    property int maxPostLength: 300
    property int maxImages: 4
    property list<string> images: initialImage ? [initialImage] : []
    property list<string> altTexts: initialImage ? [""] : []
    property bool pickingImage: false

    // Reply-to
    property basicprofile replyToAuthor
    property string replyToPostUri: ""
    property string replyToPostCid: ""
    property string replyRootPostUri: ""
    property string replyRootPostCid: ""
    property string replyToPostText
    property date replyToPostDateTime

    // Quote post
    property bool openedAsQuotePost: false
    property basicprofile quoteAuthor
    property string quoteUri: ""
    property string quoteCid: ""
    property string quoteText
    property date quoteDateTime

    signal closed

    id: page
    width: parent.width
    height: parent.height
    topPadding: 10
    bottomPadding: 10

    header: Rectangle {
        width: parent.width
        height: guiSettings.headerHeight
        z: guiSettings.headerZLevel
        color: guiSettings.headerColor

        SkyButton {
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            text: qsTr("Cancel")
            onClicked: page.cancel()
        }

        Avatar {
            anchors.centerIn: parent
            height: parent.height - 10
            width: height
            avatarUrl: skywalker.avatarUrl
            onClicked: skywalker.showStatusMessage("Yes, you're gorgeous!", QEnums.STATUS_LEVEL_INFO)
            onPressAndHold: skywalker.showStatusMessage("Yes, you're really gorgeous!", QEnums.STATUS_LEVEL_INFO)
        }

        SkyButton {
            id: postButton
            anchors.rightMargin: 10
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            text: replyToPostUri ? qsTr("Reply", "verb on reply button") : qsTr("Post", "verb on post button")

            enabled: postText.graphemeLength <= maxPostLength && page.hasContent()
            onClicked: {
                postButton.enabled = false

                if (!linkCard.card) {
                    postUtils.post(postText.text, images, altTexts,
                                   replyToPostUri, replyToPostCid,
                                   replyRootPostUri, replyRootPostCid,
                                   quoteUri, quoteCid);
                } else {
                    postUtils.post(postText.text, linkCard.card,
                                   replyToPostUri, replyToPostCid,
                                   replyRootPostUri, replyRootPostCid,
                                   quoteUri, quoteCid)
                }
            }
        }
    }

    footer: Rectangle {
        id: textFooter
        width: page.width
        height: guiSettings.footerHeight
        z: guiSettings.footerZLevel
        color: guiSettings.footerColor

        ProgressBar {
            id: textLengthBar
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            from: 0
            to: Math.max(page.maxPostLength, postText.graphemeLength)
            value: postText.graphemeLength

            contentItem: Rectangle {
                width: textLengthBar.visualPosition * parent.width
                height: parent.height
                color: postText.graphemeLength <= maxPostLength ? guiSettings.buttonColor : guiSettings.errorColor
            }
        }

        SvgImage {
            x: 10
            y: height + 5
            width: 34
            height: 34
            color: page.images.length < maxImages && imageScroller.visible && !pickingImage ? guiSettings.buttonColor : guiSettings.disabledColor
            opacity: 1
            svg: svgOutline.addImage

            MouseArea {
                y: -parent.y
                width: parent.width
                height: parent.height
                enabled: page.images.length < maxImages && !pickingImage

                onClicked: {
                    if (Qt.platform.os === "android") {
                        pickingImage = postUtils.pickPhoto()
                    } else {
                        fileDialog.open()
                    }
                }
            }
        }

        Text {
            y: 10
            anchors.rightMargin: 10
            anchors.right: parent.right
            color: postText.graphemeLength <= maxPostLength ? guiSettings.textColor : guiSettings.errorColor
            text: maxPostLength - postText.graphemeLength
        }
    }

    Flickable {
        id: flick
        anchors.fill: parent
        clip: true
        contentWidth: postText.width
        contentHeight: quoteColumn.y + (quoteColumn.visible ? quoteColumn.height : 0)
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds

        onHeightChanged: ensureVisible(postText.cursorRectangle)

        // Reply-to
        Rectangle {
            anchors.fill: replyToColumn
            border.width: 2
            border.color: guiSettings.borderColor
            color: guiSettings.postHighLightColor
            visible: replyToColumn.visible
        }
        QuotePost {
            id: replyToColumn
            width: parent.width - 20
            anchors.horizontalCenter: parent.horizontalCenter
            author: replyToAuthor
            postText: replyToPostText
            postDateTime: replyToPostDateTime
            visible: replyToPostUri
        }

        // Post text
        TextEdit {
            property int graphemeLength: 0

            id: postText
            y: replyToColumn.visible ? replyToColumn.height + 5 : 0
            width: page.width
            leftPadding: 10
            rightPadding: 10
            textFormat: TextEdit.PlainText
            wrapMode: TextEdit.Wrap
            font.pointSize: guiSettings.scaledFont(9/8)
            color: guiSettings.textColor
            clip: true
            focus: true
            text: initialText

            onCursorRectangleChanged: {
                let editMentionY = postUtils.editMentionCursorY
                let cursorY = cursorRectangle.y

                if (postUtils.editMention.length > 0 && editMentionY != cursorY)
                    postUtils.editMention = ""

                flick.ensureVisible(cursorRectangle)
            }

            onTextChanged: {
                highlightFacets()
                updateGraphemeLength()
            }

            onPreeditTextChanged: updateGraphemeLength()

            function highlightFacets() {
                postUtils.extractMentionsAndLinks(postText.text,
                        postText.preeditText, cursorPosition)
            }

            function updateGraphemeLength() {
                graphemeLength = postUtils.graphemeLength(postText.text) +
                        postUtils.graphemeLength(preeditText) -
                        postUtils.getLinkShorteningReduction()
            }

            Text {
                anchors.fill: parent
                leftPadding: postText.leftPadding
                rightPadding: postText.rightPadding
                font.pointSize: postText.font.pointSize
                color: guiSettings.placeholderTextColor
                text: qsTr("Say something nice")
                visible: postText.graphemeLength === 0
            }
        }

        // Typeahead matches on parital mention
        SimpleAuthorListView {
            id: typeaheadView
            y: postText.y + postText.cursorRectangle.y + postText.cursorRectangle.height + 5
            z: 10
            width: page.width
            height: page.footer.y - y - 5
            model: searchUtils.authorTypeaheadList
            visible: postUtils.editMention.length > 0

            onVisibleChanged: {
                if (!visible)
                    searchUtils.authorTypeaheadList = []
            }

            onAuthorClicked: (profile) => {
                const textBefore = postText.text.slice(0, postText.cursorPosition)
                const textBetween = postText.preeditText
                const textAfter = postText.text.slice(postText.cursorPosition)
                const fullText = textBefore + textBetween + textAfter
                const mentionStartIndex = postUtils.getEditMentionIndex()
                const mentionEndIndex = mentionStartIndex + postUtils.editMention.length
                postText.clear() // also clears the preedit buffer

                // Add space and move the cursor 1 postion beyond the end
                // of then mention. That causes the typeahead list to disappear.
                const newText = fullText.slice(0, mentionStartIndex) + profile.handle + ' ' + fullText.slice(mentionEndIndex)
                postText.text = newText
                postText.cursorPosition = mentionStartIndex + profile.handle.length + 1
            }
        }

        // Image attachments
        ScrollView {
            property int imgWidth: 240

            id: imageScroller
            height: visible && page.images.length > 0 ? 180 : 0
            width: page.width
            anchors.topMargin: 10
            horizontalPadding: 10
            anchors.top: postText.bottom
            contentWidth: imageRow.width
            contentHeight: height
            visible: !linkCard.visible

            Row {
                id: imageRow
                width: page.images.length * imageScroller.imgWidth + (page.images.length - 1) * spacing
                spacing: 10

                Repeater {
                    model: page.images

                    Image {
                        required property string modelData
                        required property int index

                        width: imageScroller.imgWidth
                        height: imageScroller.height
                        fillMode: Image.PreserveAspectCrop
                        autoTransform: true
                        source: modelData

                        onStatusChanged: {
                            if (status === Image.Error)
                            {
                                statusPopup.show(qsTr("Cannot load image"), QEnums.STATUS_LEVEL_ERROR);
                                page.altTexts.splice(index, 1)
                                page.images.splice(index, 1)
                            }
                        }

                        SkyButton {
                            flat: hasAltText(index)
                            text: hasAltText(index) ? qsTr("ALT") : qsTr("+ALT", "add alternative text button")
                            onClicked: editAltText(index)
                        }

                        SvgButton {
                            x: parent.width - width
                            height: width
                            iconColor: guiSettings.buttonTextColor
                            Material.background: guiSettings.buttonColor
                            svg: svgOutline.close
                            onClicked: {
                                page.altTexts.splice(index, 1)
                                page.images.splice(index, 1)
                            }
                        }
                    }
                }
            }
        }

        // Link card attachment
        LinkCardView {
            property var card: null

            id: linkCard
            x: 10
            width: page.width - 20
            height: card ? columnHeight : 0
            anchors.top: imageScroller.bottom
            uri: card ? card.link : ""
            title: card ? card.title : ""
            description: card ? card.description : ""
            thumbUrl: card ? card.thumb : ""
            visible: card

            function show(card) {
                linkCard.card = card
            }

            function hide() {
                linkCard.card = null
            }

            SvgButton {
                x: parent.width - width
                height: width
                iconColor: guiSettings.buttonTextColor
                Material.background: guiSettings.buttonColor
                svg: svgOutline.close
                onClicked: linkCard.hide()
            }
        }

        // Quote post
        Rectangle {
            anchors.fill: quoteColumn
            border.width: 2
            border.color: guiSettings.borderColor
            color: guiSettings.postHighLightColor
            visible: quoteColumn.visible
        }
        QuotePost {
            id: quoteColumn
            width: parent.width - 20
            anchors.top: linkCard.bottom
            anchors.topMargin: 5
            anchors.horizontalCenter: parent.horizontalCenter
            author: quoteAuthor
            postText: quoteText
            postDateTime: quoteDateTime
            visible: quoteUri
        }

        function ensureVisible(cursor) {
            let cursorY = cursor.y + postText.y

            if (contentY >= cursorY)
                contentY = cursorY;
            else if (contentY + height <= cursorY + cursor.height)
                contentY = cursorY + cursor.height - height;
        }
    }

    StatusPopup {
        id: statusPopup
    }

    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        running: false
    }

    FileDialog {
        id: fileDialog
        currentFolder: StandardPaths.standardLocations(StandardPaths.PicturesLocation)[0]
        nameFilters: ["Image files (*.jpg *.jpeg *.png *.webp *.gif)"]
        onAccepted: {
            let fileName = selectedFile.toString()
            if (!fileName.startsWith("file://"))
                fileName = "file://" + fileName

            photoPicked(fileName)
        }
    }

    LinkCardReader {
        id: linkCardReader
        onLinkCard: (card) => {
                        console.debug("Got card:", card.link, card.title, card.thumb)
                        console.debug(card.description)
                        linkCard.show(card)
                    }
    }

    Timer {
        id: linkCardTimer
        interval: 1000
        onTriggered: linkCardReader.getLinkCard(postUtils.firstWebLink)
    }

    PostUtils {
        property double editMentionCursorY: 0

        id: postUtils
        skywalker: page.skywalker

        onPostOk: postDone()
        onPostFailed: (error) => page.postFailed(error)
        onPostProgress: (msg) => page.postProgress(msg)

        onPhotoPicked: (fileName) => {
            pickingImage = false
            page.photoPicked("file://" + fileName)
        }

        onPhotoPickCanceled: {
            pickingImage = false
        }

        onEditMentionChanged: {
            console.debug(editMention)
            editMentionCursorY = postText.cursorRectangle.y
            typeaheadSearchTimer.start()
        }

        onFirstWebLinkChanged: {
            linkCard.hide()

            if (firstWebLink) {
                linkCardTimer.start()
            } else {
                linkCardTimer.stop()
            }
        }

        onFirstPostLinkChanged: {
            if (page.openedAsQuotePost)
                return

            quoteUri = ""
            if (firstPostLink) {
                postUtils.getQuotePost(firstPostLink)
            }
        }

        onQuotePost: (uri, cid, text, author, timestamp) => {
                if (!firstPostLink)
                    return

                page.quoteUri = uri
                page.quoteCid = cid
                page.quoteText = text
                page.quoteAuthor = author
                page.quoteDateTime = timestamp
            }
    }

    Timer {
        id: typeaheadSearchTimer
        interval: 500
        onTriggered: {
            if (postUtils.editMention.length > 0)
                searchUtils.searchAuthorsTypeahead(postUtils.editMention, 10)
        }
    }

    SearchUtils {
        id: searchUtils
        skywalker: page.skywalker

        Component.onDestruction: {
            // The destuctor of SearchUtils is called too late by the QML engine
            // Remove models now before the Skywalker object is destroyed.
            searchUtils.removeModels()
        }
    }

    Timer {
        id: focusTimer
        interval: 100
        onTriggered: {
            postText.cursorPosition = initialText.length
            flick.ensureVisible(Qt.rect(0, 0, postText.width, postText.height))
            postText.focus = true
        }
    }

    GuiSettings {
        id: guiSettings
    }

    function postFailed(error) {
        busyIndicator.running = false
        statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
        postButton.enabled = true
    }

    function postProgress(msg) {
        busyIndicator.running = true
        statusPopup.show(msg, QEnums.STATUS_LEVEL_INFO)
    }

    function hasAltText(index) {
        if (index >= altTexts.length)
            return false

        return altTexts[index].length > 0
    }

    function photoPicked(fileName) {
        console.debug("IMAGE:", fileName)
        page.altTexts.push("")
        page.images.push(fileName)
        let scrollBar = imageScroller.ScrollBar.horizontal
        scrollBar.position = 1.0 - scrollBar.size
    }

    function addSharedPhoto(fileName) {
        if (page.images.length >= page.maxImages) {
            statusPopup.show(qsTr(`Post already has ${page.maxImages} images attached.`), QEnums.STATUS_LEVEL_INFO, 30)
            return
        }

        photoPicked(fileName)
    }

    function postDone() {
        busyIndicator.running = false
        page.closed()
    }

    function hasContent() {
        return postText.graphemeLength > 0 || page.images.length > 0
    }

    function cancel() {
        if (!hasContent()) {
            page.closed()
            return
        }

        guiSettings.askYesNoQuestion(
                    page,
                    qsTr("Do you really want to discard your draft post?"),
                    () => page.closed())
    }

    function editAltText(index) {
        let component = Qt.createComponent("AltTextEditor.qml")
        let altPage = component.createObject(page, {
                imgSource: page.images[index],
                text: page.altTexts[index] })
        altPage.onAltTextChanged.connect((text) => {
                page.altTexts[index] = text
                root.popStack()
        })
        root.pushStack(altPage)
    }

    Connections {
        target: Qt.inputMethod

        // Resize the footer when the Android virtual keyboard is shown
        function onKeyboardRectangleChanged() {
            if (Qt.inputMethod.keyboardRectangle.y > 0) {
                const keyboardY = Qt.inputMethod.keyboardRectangle.y  / Screen.devicePixelRatio
                textFooter.height = guiSettings.footerHeight + (parent.height - keyboardY)
            }
            else {
                textFooter.height = guiSettings.footerHeight
            }
        }
    }

    Component.onCompleted: {
        // Wait a bit for the window to render.
        // Then make sue the text field is in the visible area.
        focusTimer.start()
        postUtils.setHighlightDocument(postText.textDocument, guiSettings.linkColor)
    }
}
