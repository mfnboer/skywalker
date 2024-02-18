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

    // Reply restrictions
    property bool restrictReply: false
    property bool allowReplyMentioned: false
    property bool allowReplyFollowing: false
    property list<int> allowListIndexes: [0, 1, 2]
    property list<bool> allowLists: [false, false, false]
    property int restrictionsListModelId: -1

    // Content warnings
    property bool cwSuggestive: false
    property bool cwNudity: false
    property bool cwPorn: false
    property bool cwGore: false

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

    readonly property string userDid: skywalker.getUserDid()
    readonly property bool requireAltText: skywalker.getUserSettings().getRequireAltText(userDid)

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

            Accessible.role: Accessible.Button
            Accessible.name: qsTr("cancel posting")
            Accessible.onPressAction: clicked()
        }

        Avatar {
            anchors.centerIn: parent
            height: parent.height - 10
            width: height
            avatarUrl: skywalker.avatarUrl
            onClicked: skywalker.showStatusMessage(qsTr("Yes, you're gorgeous!"), QEnums.STATUS_LEVEL_INFO)
            onPressAndHold: skywalker.showStatusMessage(qsTr("Yes, you're really gorgeous!"), QEnums.STATUS_LEVEL_INFO)

            Accessible.role: Accessible.Button
            Accessible.name: qsTr("your avatar")
            Accessible.onPressAction: clicked()
        }

        SkyButton {
            id: postButton
            anchors.rightMargin: 10
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            text: replyToPostUri ? qsTr("Reply", "verb on post composition") : qsTr("Post", "verb on post composition")

            enabled: postText.graphemeLength <= postText.maxLength && page.hasContent() && checkAltText()
            onClicked: sendPost()

            Accessible.role: Accessible.Button
            Accessible.name: replyToPostUri ? qsTr("send reply") : qsTr("send post")
            Accessible.onPressAction: if (enabled) clicked()

            function sendPost() {
                postButton.enabled = false

                const qUri = getQuoteUri()
                const qCid = getQuoteCid()
                const labels = getContentLabels()

                if (linkCard.card) {
                    postUtils.post(postText.text, linkCard.card,
                                   replyToPostUri, replyToPostCid,
                                   replyRootPostUri, replyRootPostCid,
                                   qUri, qCid, labels)
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
                                   qUri, qCid, labels)
                } else {
                    postUtils.post(postText.text, images, altTexts,
                                   replyToPostUri, replyToPostCid,
                                   replyRootPostUri, replyRootPostCid,
                                   qUri, qCid, labels);
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

            Accessible.role: Accessible.Link
            Accessible.name: getRestrictionsSpeech()
            Accessible.onPressAction: page.addReplyRestrictions()

            function getRestrictionsSpeech() {
                const speech = unicodeFonts.toPlainText(restrictionText.text)
                return qsTr(`${speech}, press to change reply restrictions`)
            }

            SvgImage {
                id: restrictionIcon
                x: 10
                y: height + 3
                width: 20
                height: 20
                color: guiSettings.linkColor
                svg: restrictReply ? svgOutline.replyRestrictions : svgOutline.noReplyRestrictions

                Accessible.ignored: true
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

                Accessible.ignored: true

                function getRestrictionText() {
                    if (!restrictReply)
                        return qsTr("Everyone can reply")

                    let restrictionList = []

                    if (allowReplyMentioned && allowReplyFollowing)
                        restrictionList.push(qsTr("mentioned and followed users"))
                    else if (allowReplyMentioned)
                        restrictionList.push(qsTr("mentioned users"))
                    else if (allowReplyFollowing)
                        restrictionList.push(qsTr("followed users"))

                    let listNames = []

                    for (let i = 0; i < allowLists.length; ++i) {
                        if (allowLists[i]) {
                            let model = skywalker.getListListModel(restrictionsListModelId)
                            const listView = model.getEntry(allowListIndexes[i])
                            const listName = unicodeFonts.toCleanedHtml(listView.name)
                            listNames.push(`<b>${listName}</b>`)
                        }
                    }

                    if (listNames.length > 0) {
                        const names = guiSettings.toWordSequence(listNames)
                        restrictionList.push(qsTr(`members of ${names}`))
                    }

                    if (restrictionList.length === 0)
                        return qsTr("Replies disabled")

                    const restrictedListText = guiSettings.toWordSequence(restrictionList)
                    return qsTr(`Only ${restrictedListText} can reply`)
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

            Rectangle {
                y: -parent.height
                width: parent.width
                height: parent.height
                color: "transparent"

                Accessible.role: Accessible.Button
                Accessible.name: qsTr("add picture")
                Accessible.onPressAction: if (parent.mustEnable()) parent.selectImage()
            }

            function mustEnable() {
                return page.images.length < maxImages && imageScroller.visible && !pickingImage
            }

            function selectImage() {
                if (Qt.platform.os === "android") {
                    pickingImage = postUtils.pickPhoto()
                } else {
                    fileDialog.open()
                }
            }

            MouseArea {
                y: -parent.height
                width: parent.width
                height: parent.height
                enabled: addImage.mustEnable()
                onClicked: addImage.selectImage()
            }
        }

        SvgImage {
            property var tenorSearchView: null

            id: addGif
            x: addImage.x + addImage.width + 10
            y: height + 5 + restrictionRow.height + footerSeparator.height
            width: 34
            height: 34
            color: addGif.mustEnable() ? guiSettings.buttonColor : guiSettings.disabledColor
            opacity: 1
            svg: svgOutline.addGif

            Rectangle {
                y: -parent.height
                width: parent.width
                height: parent.height
                color: "transparent"

                Accessible.role: Accessible.Button
                Accessible.name: qsTr("add GIF")
                Accessible.onPressAction: if (parent.mustEnable()) parent.selectGif()
            }

            function mustEnable() {
                return !gifAttachment.visible && !linkCard.visible && page.images.length === 0
            }

            function selectGif() {
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

            MouseArea {
                property var tenorSearchView: null

                y: -parent.height
                width: parent.width
                height: parent.height
                enabled: addGif.mustEnable()
                onClicked: addGif.selectGif()
            }
        }

        ComboBox {
            property bool pressedWithVirtualKeyboard: false

            id: fontSelector
            x: addGif.x + addGif.width + 15
            y: 5 + restrictionRow.height + footerSeparator.height
            height: 34
            model: ["Normal", "𝗕𝗼𝗹𝗱", "𝘐𝘵𝘢𝘭𝘪𝘤", "S̶t̶r̶i̶k̶e̶", "𝙼𝚘𝚗𝚘", "Sᴍᴀʟʟ ᴄᴀᴘs", "𝓒𝓾𝓻𝓼𝓲𝓿𝓮", "Ｗｉｄｅ", "Ⓑⓤⓑⓑⓛⓔ", "🅂🅀🅄🄰🅁🄴"]

            background: Rectangle {
                implicitWidth: 150
                implicitHeight: 34
                border.color: guiSettings.buttonColor
                border.width: 2
                color: "transparent"
            }

            onPressedChanged: {
                // On Android, a press on the combobox makes the virtual keyboard to close.
                // This causes to popup to close or not open at all. Open it after the
                // keyboard has closed.
                if (pressed && Qt.inputMethod.keyboardRectangle.y > 0)
                    pressedWithVirtualKeyboard = true
            }

            popup.onClosed: postText.forceActiveFocus()

            Accessible.ignored: true

            function virtualKeyboardClosed() {
                if (pressedWithVirtualKeyboard) {
                    pressedWithVirtualKeyboard = false

                    if (!popup.opened)
                        popup.open()
                }
            }

            Component.onCompleted: {
                fontSelector.contentItem.color = guiSettings.buttonColor
                fontSelector.indicator.color = guiSettings.buttonColor
            }
        }

        SvgImage {
            id: contentWarningIcon
            anchors.left: fontSelector.right
            anchors.leftMargin: 15
            y: height + 5 + restrictionRow.height + footerSeparator.height
            width: 34
            height: 34
            color: guiSettings.linkColor
            svg: hasContentWarning() ? svgOutline.hideVisibility : svgOutline.visibility
            visible: hasImageContent()

            Rectangle {
                y: -parent.height
                width: parent.width
                height: parent.height
                color: "transparent"

                Accessible.role: Accessible.Button
                Accessible.name: qsTr("add content warning")
                Accessible.onPressAction: if (hasImageContent()) page.addContentWarning()
            }

            MouseArea {
                y: -parent.height
                width: parent.width
                height: parent.height
                enabled: hasImageContent()
                onClicked: page.addContentWarning()
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
            ellipsisBackgroundColor: guiSettings.postHighLightColor
            visible: replyToPostUri
        }

        // Post text
        TextEdit {
            readonly property int maxLength: 300
            property int graphemeLength: 0
            property bool textChangeInProgress: false

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

            Accessible.role: Accessible.EditableText
            Accessible.name: text ? text : qsTr("Say something nice")
            Accessible.description: Accessible.name
            Accessible.editable: true
            Accessible.multiLine: true

            onCursorRectangleChanged: {
                let editMentionY = postUtils.editMentionCursorY
                let cursorY = cursorRectangle.y

                if (postUtils.editMention.length > 0 && editMentionY != cursorY)
                    postUtils.editMention = ""

                flick.ensureVisible(cursorRectangle)
            }

            onTextChanged: {
                if (textChangeInProgress)
                    return

                textChangeInProgress = true
                highlightFacets()

                const added = updateGraphemeLength()
                if (added > 0)
                    updateTextTimer.set(added)

                textChangeInProgress = false
            }

            onPreeditTextChanged: {
                if (textChangeInProgress)
                    return

                const added = updateGraphemeLength()
                if (added > 0)
                    updateTextTimer.set(added)
            }

            // Text can only be changed outside onPreeditTextChanged.
            // This timer makes the call to applyFont async.
            Timer {
                property int numChars: 1

                id: updateTextTimer
                interval: 0
                onTriggered: {
                    postText.textChangeInProgress = true
                    postText.applyFont(numChars)
                    postText.textChangeInProgress = false
                }

                function set(num) {
                    numChars = num
                    start()
                }
            }

            function applyFont(numChars) {
                const modifiedTillCursor = postUtils.applyFontToLastTypedChars(
                                             postText.text, postText.preeditText,
                                             postText.cursorPosition, numChars,
                                             fontSelector.currentIndex)

                if (modifiedTillCursor) {
                    const fullText = modifiedTillCursor + postText.text.slice(postText.cursorPosition)
                    postText.clear()
                    postText.text = fullText
                    postText.cursorPosition = modifiedTillCursor.length
                }
            }

            function highlightFacets() {
                postUtils.extractMentionsAndLinks(postText.text,
                        postText.preeditText, cursorPosition)
            }

            function updateGraphemeLength() {
                const prevGraphemeLength = graphemeLength
                const linkShorteningReduction = postUtils.getLinkShorteningReduction();

                graphemeLength = unicodeFonts.graphemeLength(postText.text) +
                        unicodeFonts.graphemeLength(preeditText) -
                        linkShorteningReduction

                postUtils.setHighLightMaxLength(postText.maxLength + linkShorteningReduction)

                return graphemeLength - prevGraphemeLength
            }

            Text {
                id: placeHolderText
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

            Accessible.name: qsTr("you can select a user from the list below")
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

                        Accessible.role: Accessible.StaticText
                        Accessible.name: qsTr(`picture ${(index + 1)}: `) + (hasAltText(index) ? altTexts[index] : "no alt text")

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

                            Accessible.role: Accessible.Button
                            Accessible.name: hasAltText(index) ? qsTr(`edit alt text for picture ${(index + 1)}`) : qsTr(`add alt text to picture ${(index + 1)}`)
                            Accessible.onPressAction: clicked()
                        }

                        SvgButton {
                            x: parent.width - width
                            height: width
                            svg: svgOutline.close
                            onClicked: page.removeImage(index)

                            Accessible.role: Accessible.Button
                            Accessible.name: qsTr(`remove picture ${(index + 1)}`)
                            Accessible.onPressAction: clicked()
                        }

                        SkyLabel {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                            backgroundColor: guiSettings.errorColor
                            horizontalAlignment: Text.AlignHCenter
                            color: "white"
                            text: "ALT text missing"
                            visible: requireAltText && !hasAltText(index)

                            Accessible.role: Accessible.StaticText
                            Accessible.name: qsTr(`picture ${(index + 1)}: `) + text
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

            Accessible.role: Accessible.StaticText
            Accessible.name: qsTr("GIF image")

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

                Accessible.role: Accessible.Button
                Accessible.name: qsTr("remove GIF image")
                Accessible.onPressAction: clicked()
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

            Accessible.role: Accessible.StaticText
            Accessible.name: getSpeech()

            function getSpeech() {
                if (!card)
                    return ""

                const hostname = new URL(card.link).hostname
                return qsTr("link card: ") + card.title + "\n\nfrom: " + hostname + "\n\n" + card.description
            }

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
            ellipsisBackgroundColor: guiSettings.postHighLightColor
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
                postUtils.addThreadgate(uri, page.allowReplyMentioned, page.allowReplyFollowing, page.getReplyRestrictionListUris())
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
            postText.forceActiveFocus()
        }

        onPhotoPickFailed: (error) => {
            pickingImage = false
            statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
            postText.forceActiveFocus()
        }

        onPhotoPickCanceled: {
            pickingImage = false
            postText.forceActiveFocus()
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

    UnicodeFonts {
        id: unicodeFonts
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
        interval: 200
        onTriggered: {
            postText.cursorPosition = initialText.length
            flick.ensureVisible(Qt.rect(0, 0, postText.width, postText.height))
            postText.forceActiveFocus()
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

    function checkAltText() {
        if (!requireAltText)
            return true

        for (let i = 0; i < images.length; ++i)
        {
            if (!hasAltText(i))
                return false
        }

        return true
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
        if (restrictionsListModelId < 0) {
            restrictionsListModelId = skywalker.createListListModel(QEnums.LIST_TYPE_ALL, QEnums.LIST_PURPOSE_CURATE, userDid)
            skywalker.getListList(restrictionsListModelId)
        }

        let component = Qt.createComponent("AddReplyRestrictions.qml")
        let restrictionsPage = component.createObject(page, {
                restrictReply: page.restrictReply,
                allowMentioned: page.allowReplyMentioned,
                allowFollowing: page.allowReplyFollowing,
                allowLists: page.allowLists,
                allowListIndexes: page.allowListIndexes,
                listModelId: page.restrictionsListModelId
        })
        restrictionsPage.onAccepted.connect(() => {
                page.restrictReply = restrictionsPage.restrictReply
                page.allowReplyMentioned = restrictionsPage.allowMentioned
                page.allowReplyFollowing = restrictionsPage.allowFollowing
                page.allowLists = restrictionsPage.allowLists
                page.allowListIndexes = restrictionsPage.allowListIndexes
                restrictionsPage.destroy()
        })
        restrictionsPage.onRejected.connect(() => restrictionsPage.destroy())
        restrictionsPage.open()
    }

    function getReplyRestrictionListUris() {
        let uris = []

        for (let i = 0; i < allowLists.length; ++i) {
            if (allowLists[i]) {
                let model = skywalker.getListListModel(restrictionsListModelId)
                const listView = model.getEntry(allowListIndexes[i])
                uris.push(listView.uri)
            }
        }

        return uris
    }

    function addContentWarning() {
        let component = Qt.createComponent("AddContentWarning.qml")
        let cwPage = component.createObject(page, {
                suggestive: cwSuggestive,
                nudity: cwNudity,
                porn: cwPorn,
                gore: cwGore
        })
        cwPage.onAccepted.connect(() => {
                cwSuggestive = cwPage.suggestive
                cwNudity = cwPage.nudity
                cwPorn = cwPage.porn
                cwGore = cwPage.gore
                cwPage.destroy()
        })
        cwPage.onRejected.connect(() => cwPage.destroy())
        cwPage.open()
    }

    function hasImageContent() {
        return gifAttachment.gif ||
                (linkCard.card && linkCard.card.thumb) ||
                page.images.length > 0
    }

    function hasContentWarning() {
        return hasImageContent() && (cwSuggestive || cwNudity || cwPorn || cwGore)
    }

    function getContentLabels() {
        let labels = []

        if (!hasImageContent())
            return labels

        if (cwSuggestive)
            labels.push("sexual")
        if (cwNudity)
            labels.push("nudity")
        if (cwPorn)
            labels.push("porn")
        if (cwGore)
            labels.push("gore")

        return labels
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
                fontSelector.virtualKeyboardClosed()
            }
        }
    }

    Component.onDestruction: {
        page.images.forEach((value, index, array) => { postUtils.dropPhoto(value); })

        if (restrictionsListModelId >= 0)
            skywalker.removeListListModel(restrictionsListModelId)
    }

    Component.onCompleted: {
        // Wait a bit for the window to render.
        // Then make sue the text field is in the visible area.
        focusTimer.start()
        postUtils.setHighlightDocument(postText.textDocument, guiSettings.linkColor,
                                       postText.maxLength, guiSettings.textLengthExceededColor)
    }
}
