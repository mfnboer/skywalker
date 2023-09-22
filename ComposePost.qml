import QtCore
import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import skywalker

Page {
    required property var skywalker
    property int maxPostLength: 300
    property int maxImages: 4
    property list<string> images

    // Reply-to
    property basicprofile replyToAuthor
    property string replyToPostUri: ""
    property string replyToPostCid: ""
    property string replyRootPostUri: ""
    property string replyRootPostCid: ""
    property string replyToPostText
    property date replyToPostDateTime

    // Quote post
    property basicprofile quoteAuthor
    property string quoteUri: ""
    property string quoteCid: ""
    property string quoteText
    property date quoteDateTime

    signal closed

    id: page
    topPadding: 10
    bottomPadding: 10

    header: Rectangle {
        width: parent.width
        height: guiSettings.headerHeight
        z: guiSettings.headerZLevel
        color: guiSettings.headerColor

        Button {
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            Material.background: guiSettings.buttonColor
            contentItem: Text {
                color: guiSettings.buttonTextColor
                text: qsTr("Cancel")
            }

            onClicked: page.closed()
        }

        Button {
            id: postButton
            anchors.rightMargin: 10
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            Material.background: "blue"

            contentItem: Text {
                color: "white"
                text: replyToPostUri ? qsTr("Reply", "verb on reply button") : qsTr("Post", "verb on post button")
            }

            enabled: postText.textLength() <= maxPostLength && (postText.textLength() > 0 || page.images.length > 0)
            onClicked: {
                postButton.enabled = false

                if (!linkCard.card) {
                    postUtils.post(postText.text, images,
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
        height: guiSettings.footerHeight + Qt.inputMethod.keyboardRectangle.height / Screen.devicePixelRatio
        z: guiSettings.footerZLevel
        color: guiSettings.footerColor

        ProgressBar {
            id: textLengthBar
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            from: 0
            to: page.maxPostLength
            value: postText.graphemeLength() - postUtils.getLinkShorteningReduction()

            contentItem: Rectangle {
                width: textLengthBar.visualPosition * parent.width
                height: parent.height
                color: textLengthBar.value <= maxPostLength ? "blue" : "red"
            }
        }

        SvgImage {
            x: 10
            y: height + 5
            width: 34
            height: 34
            id: homeButton
            color: page.images.length < maxImages && imageScroller.visible ? "blue" : "lightgrey"
            svg: svgOutline.addImage

            MouseArea {
                y: -parent.y
                width: parent.width
                height: parent.height
                visible: page.images.length < maxImages

                onClicked: {
                    if (Qt.platform.os === "android")
                        postUtils.pickPhoto()
                    else
                        fileDialog.open()
                }
            }
        }

        Text {
            y: 10
            anchors.rightMargin: 10
            anchors.right: parent.right
            color: postText.textLength() <= maxPostLength ? Material.foreground : "red"
            text: maxPostLength - textLengthBar.value
        }
    }

    Flickable {
        id: flick
        anchors.fill: parent
        clip: true
        contentWidth: postText.width
        contentHeight: quoteColumn.y + (quoteColumn.visible ? quoteColumn.height : 0)
        flickableDirection: Flickable.VerticalFlick

        // Reply-to
        Rectangle {
            anchors.fill: replyToColumn
            border.width: 2
            color: "azure"
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
        TextArea {
            property string highlightedText: ""

            id: postText
            y: replyToColumn.visible ? replyToColumn.height + 5 : 0
            width: page.width
            leftPadding: 10
            rightPadding: 10
            placeholderText: textLength() === 0 ? "Say something nice" : ""
            placeholderTextColor: "grey"
            textFormat: TextEdit.PlainText
            wrapMode: TextEdit.Wrap
            font.pointSize: guiSettings.scaledFont(9/8)
            clip: true
            focus: true
            color: "transparent" // HACK: the highlighted text is show by displayText
            background: Rectangle { border.color: "transparent" }

            onCursorRectangleChanged: flick.ensureVisible(cursorRectangle)

            function highlightFacets() {
                highlightedText = postUtils.highlightMentionsAndLinks(postText.text,
                        postText.preeditText, cursorPosition, guiSettings.linkColor)
            }

            onTextChanged: highlightFacets()

            function textLength() {
                return length + preeditText.length
            }

            function graphemeLength() {
                return postUtils.graphemeLength(postText.text) + postUtils.graphemeLength(preeditText)
            }

            // This Text element overlays the invisible TextArea to show the highlighted
            // content.
            Text {
                id: displayText
                anchors.fill: parent
                leftPadding: postText.leftPadding
                rightPadding: postText.rightPadding
                textFormat: TextEdit.RichText
                wrapMode: postText.wrapMode
                font.pointSize: postText.font.pointSize
                text: postText.highlightedText
            }
        }

        // Image attachments
        ScrollView {
            property int imgWidth: 240

            id: imageScroller
            height: visible && page.images.length > 0 ? 180 : 0
            width: page.width
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
                        source: "file://" + modelData

                        onStatusChanged: {
                            if (status === Image.Error)
                            {
                                statusPopup.show(qsTr("Cannot load image"), QEnums.STATUS_LEVEL_ERROR);
                                page.images.splice(index, 1)
                            }
                        }

                        RoundButton {
                            Material.background: "black"
                            contentItem: Text {
                                anchors.horizontalCenter: parent.horizontalCenter
                                color: "white"
                                font.pointSize: guiSettings.scaledFont(7/8)
                                font.bold: true
                                text: qsTr("+ALT", "add alternative text button")
                            }
                        }

                        SvgButton {
                            x: parent.width - width
                            height: width
                            iconColor: "white"
                            Material.background: "black"
                            opacity: 1
                            svg: svgOutline.close
                            onClicked: page.images.splice(index, 1)
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
                iconColor: "white"
                Material.background: "black"
                opacity: 1
                svg: svgOutline.close
                onClicked: linkCard.hide()
            }
        }

        // Quote post
        Rectangle {
            anchors.fill: quoteColumn
            border.width: 2
            color: "azure"
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

    FileDialog {
        id: fileDialog
        currentFolder: StandardPaths.standardLocations(StandardPaths.PicturesLocation)[0]
        nameFilters: ["Image files (*.jpg *.jpeg *.png)"]
        onAccepted: {
            let fileName = selectedFile.toString()
            if (fileName.startsWith("file://"))
                fileName = fileName.substr(7)

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
        id: postUtils
        skywalker: page.skywalker

        onPostOk: postDone()
        onPostFailed: (error) => page.postFailed(error)
        onPostProgress: (msg) => page.postProgress(msg)
        onPhotoPicked: (fileName) => page.photoPicked(fileName)

        onFirstWebLinkChanged: {
            if (firstWebLink)
            {
                linkCard.hide()
                linkCardTimer.start()
            }
            else
            {
                linkCard.hide()
                linkCardTimer.stop()
            }
        }
    }

    GuiSettings {
        id: guiSettings
    }

    function postFailed(error) {
        statusPopup.show(error, QEnums.STATUS_LEVEL_ERROR)
        postButton.enabled = true
    }

    function postProgress(msg) {
        statusPopup.show(msg, QEnums.STATUS_LEVEL_INFO)
    }

    function photoPicked(fileName) {
        console.debug("IMAGE:", fileName)
        page.images.push(fileName)
        let scrollBar = imageScroller.ScrollBar.horizontal
        scrollBar.position = 1.0 - scrollBar.size
    }

    function postDone() {
        page.closed()
    }

    Component.onCompleted: {
        postText.focus = true
    }
}
