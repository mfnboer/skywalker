import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window 2.2
import skywalker

Page {
    required property var skywalker
    property string initialText
    property string initialImage

    property int maxImages: 4
    property list<string> images: initialImage ? [initialImage] : []
    property list<string> altTexts: initialImage ? [""] : []
    property bool pickingImage: false
    property bool restrictReply: false
    property bool allowReplyMentioned: false
    property bool allowReplyFollowing: false

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

    // Quote feed
    property generatorview quoteFeed
    property generatorview nullFeed

    // Quote list
    property listview quoteList
    property listview nullList

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

        SvgButton {
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            svg: svgOutline.cancel
            onClicked: page.cancel()
        }

        Avatar {
            anchors.centerIn: parent
            height: parent.height - 10
            width: height
            avatarUrl: skywalker.avatarUrl
            onClicked: skywalker.showStatusMessage(qsTr("Yes, you're gorgeous!"), QEnums.STATUS_LEVEL_INFO)
            onPressAndHold: skywalker.showStatusMessage(qsTr("Yes, you're really gorgeous!"), QEnums.STATUS_LEVEL_INFO)
        }

        SkyButton {
            id: postButton
            anchors.rightMargin: 10
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            text: replyToPostUri ? qsTr("Reply", "verb on post composition") : qsTr("Post", "verb on post composition")

            enabled: postText.graphemeLength <= postText.maxLength && page.hasContent()
            onClicked: {
                postButton.enabled = false

                const qUri = getQuoteUri()
                const qCid = getQuoteCid()

                if (linkCard.card) {
                    postUtils.post(postText.text, linkCard.card,
                                   replyToPostUri, replyToPostCid,
                                   replyRootPostUri, replyRootPostCid,
                                   qUri, qCid)
                } else if (gifAttachment.gif) {
                    tenor.registerShare(gifAttachment.gif)

                    let gifCard = linkCardReader.makeLinkCard(
                            gifAttachment.gif.url,
                            gifAttachment.gif.description + " (via Tenor)\nPosted from Skywalker @skywalkerapp.bsky.social",
                            qsTr("This GIF has been posted from Skywalker for Android. " +
                                 "Get Skywalker from Google Play.") +
                                 ("\nBluesky: @skywalkerapp.bsky.social"),
                            gifAttachment.gif.imageUrl)

                    postUtils.post(postText.text, gifCard,
                                   replyToPostUri, replyToPostCid,
                                   replyRootPostUri, replyRootPostCid,
                                   qUri, qCid)
                } else {
                    postUtils.post(postText.text, images, altTexts,
                                   replyToPostUri, replyToPostCid,
                                   replyRootPostUri, replyRootPostCid,
                                   qUri, qCid);
                }
            }
        }
    }

    footer: Rectangle {
        id: textFooter
        width: page.width
        height: getFooterHeight()
        z: guiSettings.footerZLevel
        color: guiSettings.footerColor

        function getFooterHeight() {
            return guiSettings.footerHeight + (replyToPostUri ? 0 : restrictionRow.height + footerSeparator.height)
        }

        Rectangle {
            id: footerSeparator
            width: parent.width
            height: replyToPostUri ? 0 : 1
            color: guiSettings.separatorColor
            visible: !replyToPostUri
        }

        Rectangle {
            id: restrictionRow
            anchors.top: footerSeparator.top
            width: parent.width
            height: replyToPostUri ? 0 : restrictionText.height + 10
            color: "transparent"
            visible: !replyToPostUri

            SvgImage {
                id: restrictionIcon
                x: 10
                y: height + 3
                width: 20
                height: 20
                color: guiSettings.linkColor
                svg: restrictReply ? svgOutline.replyRestrictions : svgOutline.noReplyRestrictions
            }
            Text {
                id: restrictionText
                y: 5
                anchors.left: restrictionIcon.right
                anchors.right: parent.right
                leftPadding: 5
                color: guiSettings.linkColor
                font.italic: true
                font.pointSize: guiSettings.scaledFont(7/8)
                wrapMode: Text.Wrap
                text: getRestrictionText()

                function getRestrictionText() {
                    if (!restrictReply)
                        return qsTr("Everyone can reply")

                    let restrictedListText = ""

                    if (allowReplyMentioned)
                        restrictedListText = qsTr("mentioned")

                    if (allowReplyFollowing) {
                        if (restrictedListText)
                            restrictedListText += qsTr(" and ")

                        restrictedListText += qsTr("followed")
                    }

                    if (restrictedListText)
                        return qsTr(`Only ${restrictedListText} users can reply`)

                    return qsTr("Replies disabled")
                }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: page.addReplyRestrictions()
            }
        }

        TextLengthBar {
            anchors.top: restrictionRow.bottom
            textField: postText
        }

        SvgImage {
            id: addImage
            x: 10
            y: height + 5 + restrictionRow.height + footerSeparator.height
            width: 34
            height: 34
            color: addImage.mustEnable() ? guiSettings.buttonColor : guiSettings.disabledColor
            opacity: 1
            svg: svgOutline.addImage

            function mustEnable() {
                return page.images.length < maxImages && imageScroller.visible && !pickingImage
            }

            MouseArea {
                y: -parent.height
                width: parent.width
                height: parent.height
                enabled: addImage.mustEnable()

                onClicked: {
                    if (Qt.platform.os === "android") {
                        pickingImage = postUtils.pickPhoto()
                    } else {
                        fileDialog.open()
                    }
                }
            }
        }

        SvgImage {
            id: addGif
            x: addImage.x + addImage.width + 10
            y: height + 5 + restrictionRow.height + footerSeparator.height
            width: 34
            height: 34
            color: addGif.mustEnable() ? guiSettings.buttonColor : guiSettings.disabledColor
            opacity: 1
            svg: svgOutline.addGif

            function mustEnable() {
                return !gifAttachment.visible && !linkCard.visible && page.images.length === 0
            }

            MouseArea {
                property var tenorSearchView: null

                y: -parent.height
                width: parent.width
                height: parent.height
                enabled: addGif.mustEnable()

                onClicked: {
                    if (!tenorSearchView)
                    {
                        let component = Qt.createComponent("TenorSearch.qml")
                        tenorSearchView = component.createObject(root)
                        tenorSearchView.onClosed.connect(() => { root.currentStack().pop() })
                        tenorSearchView.onSelected.connect((gif) => {
                                gifAttachment.show(gif)
                                root.currentStack().pop()
                        })
                    }

                    root.pushStack(tenorSearchView)
                }
            }
        }

        TextLengthCounter {
            y: 10 + restrictionRow.height + footerSeparator.height
            anchors.rightMargin: 10
            anchors.right: parent.right
            textField: postText
        }
    }

    Flickable {
        id: flick
        anchors.fill: parent
        clip: true
        contentWidth: postText.width
        contentHeight: quoteListColumn.y + (quoteListColumn.visible ? quoteListColumn.height : 0)
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
            readonly property int maxLength: 300
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
            selectionColor: guiSettings.selectionColor
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
        AuthorTypeaheadView {
            y: postText.y + postText.cursorRectangle.y + postText.cursorRectangle.height + 5
            parentPage: page
            editText: postText
            searchUtils: searchUtils
            postUtils: postUtils
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
            visible: !linkCard.visible && !gifAttachment.visible

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
                            if (status === Image.Error){
                                statusPopup.show(qsTr("Cannot load image"), QEnums.STATUS_LEVEL_ERROR);
                                page.removeImage(index)
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
                            svg: svgOutline.close
                            onClicked: page.removeImage(index)
                        }
                    }
                }
            }
        }

        // GIF attachment
        AnimatedImage {
            property var gif: null

            id: gifAttachment
            x: 10
            width: Math.min(gif ? gif.smallSize.width : 1, page.width - 20)
            anchors.top: imageScroller.bottom
            fillMode: Image.PreserveAspectFit
            source: gif ? gif.smallUrl : ""
            visible: gif

            function show(gif) {
                gifAttachment.gif = gif
                linkCard.hide()
            }

            function hide() {
                gifAttachment.gif = null
            }

            SvgButton {
                x: parent.width - width
                height: width
                svg: svgOutline.close
                onClicked: gifAttachment.hide()
            }
        }

        // Link card attachment
        LinkCardView {
            property var card: null

            id: linkCard
            x: 10
            width: page.width - 20
            height: card ? columnHeight : 0
            anchors.top: gifAttachment.bottom
            uri: card ? card.link : ""
            title: card ? card.title : ""
            description: card ? card.description : ""
            thumbUrl: card ? card.thumb : ""
            visible: card

            function show(card) {
                linkCard.card = card
                gifAttachment.hide()
            }

            function hide() {
                linkCard.card = null
            }

            SvgButton {
                x: parent.width - width
                height: width
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

        // Quote feed
        Rectangle {
            anchors.fill: quoteFeedColumn
            border.width: 2
            border.color: guiSettings.borderColor
            color: guiSettings.postHighLightColor
            visible: quoteFeedColumn.visible
        }
        QuoteFeed {
            id: quoteFeedColumn
            width: parent.width - 20
            anchors.top: linkCard.bottom
            anchors.topMargin: 5
            anchors.horizontalCenter: parent.horizontalCenter
            feed: page.quoteFeed
            visible: !page.quoteFeed.isNull()
        }

        // Quote list
        Rectangle {
            anchors.fill: quoteListColumn
            border.width: 2
            border.color: guiSettings.borderColor
            color: guiSettings.postHighLightColor
            visible: quoteListColumn.visible
        }
        QuoteList {
            id: quoteListColumn
            width: parent.width - 20
            anchors.top: linkCard.bottom
            anchors.topMargin: 5
            anchors.horizontalCenter: parent.horizontalCenter
            list: page.quoteList
            visible: !page.quoteList.isNull()
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

    ImageFileDialog {
        id: fileDialog
        onFileSelected: (fileName) => { photoPicked(fileName) }
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

        onPostOk: (uri, cid) => {
            if (page.restrictReply)
                postUtils.addThreadgate(uri, page.allowReplyMentioned, page.allowReplyFollowing)
            else
                postDone()
        }

        onPostFailed: (error) => page.postFailed(error)

        onThreadgateOk: postDone()
        onThreadgateFailed: (error) => page.postFailed(error)

        onPostProgress: (msg) => page.postProgress(msg)

        onPhotoPicked: (imgSource) => {
            pickingImage = false
            page.photoPicked(imgSource)
        }

        onPhotoPickFailed: (error) => {
            pickingImage = false
            statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
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
            if (gifAttachment.visible)
                return

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

            page.quoteList = page.nullList
            page.quoteFeed = page.nullFeed
            quoteUri = ""

            if (firstPostLink)
                postUtils.getQuotePost(firstPostLink)
        }

        onFirstFeedLinkChanged: {
            if (page.openedAsQuotePost)
                return

            page.quoteList = page.nullList
            page.quoteFeed = page.nullFeed

            if (firstPostLink)
                return

            if (firstFeedLink)
                postUtils.getQuoteFeed(firstFeedLink)
        }

        onFirstListLinkChanged: {
            if (page.openedAsQuotePost)
                return

            page.quoteList = page.nullList

            if (firstPostLink || firstFeedLink)
                return

            if (firstListLink)
                postUtils.getQuoteList(firstListLink)
        }

        onQuotePost: (uri, cid, text, author, timestamp) => {
                if (!firstPostLink)
                    return

                page.quoteList = page.nullList
                page.quoteFeed = page.nullFeed
                page.quoteUri = uri
                page.quoteCid = cid
                page.quoteText = text
                page.quoteAuthor = author
                page.quoteDateTime = timestamp
            }

        onQuoteFeed: (feed) => {
                if (!firstFeedLink)
                    return

                if (firstPostLink)
                    return

                page.quoteList = page.nullList
                page.quoteFeed = feed
            }

        onQuoteList: (list) => {
                if (!firstListLink)
                    return

                if (firstPostLink || firstFeedLink)
                    return

                page.quoteList = list
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

    Tenor {
        id: tenor
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

    // "file://" or "image://" source
    function photoPicked(source) {
        console.debug("IMAGE:", source)
        page.altTexts.push("")
        page.images.push(source)
        let scrollBar = imageScroller.ScrollBar.horizontal
        scrollBar.position = 1.0 - scrollBar.size
    }

    function addSharedText(text) {
        if (text)
            postText.append(text)
    }

    function addSharedPhoto(source, text) {
        if (page.images.length >= page.maxImages) {
            statusPopup.show(qsTr(`Post already has ${page.maxImages} images attached.`), QEnums.STATUS_LEVEL_INFO, 30)
            postUtils.dropPhoto(source)
            return
        }

        photoPicked(source)
        addSharedText(text)
    }

    function removeImage(index) {
        postUtils.dropPhoto(page.images[index])
        page.altTexts.splice(index, 1)
        page.images.splice(index, 1)
    }

    function postDone() {
        busyIndicator.running = false
        page.closed()
    }

    function hasContent() {
        return postText.graphemeLength > 0 ||
                page.images.length > 0 ||
                linkCard.card ||
                gifAttachment.gif
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

    function addReplyRestrictions() {
        let component = Qt.createComponent("AddReplyRestrictions.qml")
        let restrictionsPage = component.createObject(page, {
                restrictReply: page.restrictReply,
                allowMentioned: page.allowReplyMentioned,
                allowFollowing: page.allowReplyFollowing
        })
        restrictionsPage.onAccepted.connect(() => {
                page.restrictReply = restrictionsPage.restrictReply
                page.allowReplyMentioned = restrictionsPage.allowMentioned
                page.allowReplyFollowing = restrictionsPage.allowFollowing
                restrictionsPage.destroy()
        })
        restrictionsPage.onRejected.connect(() => restrictionsPage.destroy())
        restrictionsPage.open()
    }

    function getQuoteUri() {
        if (quoteUri)
            return quoteUri

        if (quoteFeed.uri)
            return quoteFeed.uri

        return quoteList.uri
    }

    function getQuoteCid() {
        if (quoteCid)
            return quoteCid

        if (quoteFeed.cid)
            return quoteFeed.cid

        return quoteList.cid
    }

    Connections {
        target: Qt.inputMethod

        // Resize the footer when the Android virtual keyboard is shown
        function onKeyboardRectangleChanged() {
            if (Qt.inputMethod.keyboardRectangle.y > 0) {
                const keyboardY = Qt.inputMethod.keyboardRectangle.y  / Screen.devicePixelRatio
                textFooter.height = textFooter.getFooterHeight() + (parent.height - keyboardY)
            }
            else {
                textFooter.height = textFooter.getFooterHeight()
            }
        }
    }

    Component.onDestruction: {
        page.images.forEach((value, index, array) => { postUtils.dropPhoto(value); })
    }

    Component.onCompleted: {
        // Wait a bit for the window to render.
        // Then make sue the text field is in the visible area.
        focusTimer.start()
        postUtils.setHighlightDocument(postText.textDocument, guiSettings.linkColor)
    }
}
