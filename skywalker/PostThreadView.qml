import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

ListView {
    required property int modelId
    required property int postEntryIndex
    signal closed

    id: view
    spacing: 0
    model: skywalker.getPostThreadModel(modelId)
    flickDeceleration: guiSettings.flickDeceleration
    maximumFlickVelocity: maxFlickVelocity
    pixelAligned: guiSettings.flickPixelAligned
    ScrollIndicator.vertical: ScrollIndicator {}

    Accessible.role: Accessible.List

    header: SimpleHeader {
        height: restrictionRow.visible ? guiSettings.headerHeight + restrictionRow.height : guiSettings.headerHeight
        text: qsTr("Post thread")
        onBack: view.closed()

        Rectangle {
            id: restrictionRect
            width: parent.width
            height: restrictionRow.height + 5
            anchors.bottom: parent.bottom
            color: guiSettings.threadStartColor(root.getSkywalker().getUserSettings().threadColor)
            border.width: 1
            border.color: guiSettings.headerColor
            visible: model.replyRestriction !== QEnums.REPLY_RESTRICTION_NONE

            Accessible.role: Accessible.StaticText
            Accessible.name: unicodeFonts.toPlainText(restrictionText.text)

            Rectangle {
                id: restrictionRow
                x: guiSettings.threadBarWidth * 5 - restrictionIcon.width
                anchors.bottom: parent.bottom
                width: parent.width - x - 10
                height: restrictionText.height + 10
                color: "transparent"

                SvgImage {
                    id: restrictionIcon
                    width: 20
                    height: 20
                    color: utils.determineForegroundColor(restrictionRect.color, "black", "white")
                    svg: svgOutline.replyRestrictions
                }
                SkyCleanedText {
                    id: restrictionText
                    anchors.left: restrictionIcon.right
                    anchors.right: parent.right
                    leftPadding: 5
                    color: restrictionIcon.color
                    font.italic: true
                    font.pointSize: guiSettings.scaledFont(7/8)
                    wrapMode: Text.Wrap
                    maximumLineCount: 3
                    elide: Text.ElideRight
                    textFormat: Text.RichText
                    plainText: restrictionRow.getRestrictionText()

                    onLinkActivated: (link) => {
                        if (link.startsWith("did:")) {
                            skywalker.getDetailedProfile(link)
                        }
                        else if (link.startsWith("at:")) {
                            root.viewListByUri(link, false)
                        }
                    }

                    Accessible.ignored: true
                }

                function getRestrictionText() {
                    const replyRestriction = model.replyRestriction

                    if (replyRestriction === QEnums.REPLY_RESTRICTION_NONE)
                        return ""

                    if (replyRestriction === QEnums.REPLY_RESTRICTION_UNKNOWN)
                        return "Replies are restricted"

                    if (replyRestriction === QEnums.REPLY_RESTRICTION_NOBODY)
                        return qsTr("Replies are disabled")

                    let restrictionList = []

                    if (replyRestriction & QEnums.REPLY_RESTRICTION_MENTIONED)
                        restrictionList.push(qsTr("mentioned users"))

                    if (replyRestriction & QEnums.REPLY_RESTRICTION_FOLLOWING) {
                        const author = model.replyRestrictionAuthor
                        restrictionList.push(qsTr(`users followed by <a href="${author.did}" style="color: ${guiSettings.linkColor}; text-decoration: none">@${author.handle}</a>`))
                    }

                    if (replyRestriction & QEnums.REPLY_RESTRICTION_LIST) {
                        const lists = model.replyRestrictionLists
                        let listNames = []

                        for (let i = 0; i < lists.length; ++i) {
                            const l = lists[i]
                            const listName = unicodeFonts.toCleanedHtml(l.name)
                            listNames.push(`<a href="${l.uri}" style="color: ${guiSettings.linkColor}; text-decoration: none">${listName}</a>`)
                        }

                        const names = guiSettings.toWordSequence(listNames)
                        restrictionList.push(qsTr(`members of ${names}`))
                    }

                    if (!restrictionList) {
                        console.warn("No restrictions found.")
                        return qsTr("Replies are restricted")
                    }

                    const restrictionListText = guiSettings.toWordSequence(restrictionList)
                    return qsTr(`Replies are restricted to ${restrictionListText}`)
                }
            }
        }
    }
    headerPositioning: ListView.OverlayHeader


    footer: Rectangle {
        width: parent.width
        z: guiSettings.footerZLevel

        PostButton {
            y: -height - 10
            svg: svgOutline.reply
            overrideOnClicked: () => reply()

            Accessible.role: Accessible.Button
            Accessible.name: qsTr(`reply to ${(getReplyToAuthor().name)}`)
            Accessible.onPressAction: clicked()
        }
    }
    footerPositioning: ListView.OverlayFooter

    delegate: PostFeedViewDelegate {
        width: view.width
    }

    FlickableRefresher {}

    Timer {
        id: syncTimer
        interval: 300
        onTriggered: positionViewAtIndex(postEntryIndex, ListView.Contain)
    }

    UnicodeFonts {
        id: unicodeFonts
    }

    Utils {
        id: utils
    }

    GuiSettings {
        id: guiSettings
    }

    function getReplyToAuthor() {
        return model.getData(postEntryIndex, AbstractPostFeedModel.Author)
    }

    function reply(initialText = "", imageSource = "") {
        const postUri = model.getData(postEntryIndex, AbstractPostFeedModel.PostUri)
        const postCid = model.getData(postEntryIndex, AbstractPostFeedModel.PostCid)
        const postText = model.getData(postEntryIndex, AbstractPostFeedModel.PostText)
        const postIndexedDateTime = model.getData(postEntryIndex, AbstractPostFeedModel.PostIndexedDateTime)
        const author = getReplyToAuthor()
        const postReplyRootUri = model.getData(postEntryIndex, AbstractPostFeedModel.PostReplyRootUri)
        const postReplyRootCid = model.getData(postEntryIndex, AbstractPostFeedModel.PostReplyRootCid)
        const postLanguages = model.getData(postEntryIndex, AbstractPostFeedModel.PostLanguages)

        const lang = postLanguages.length > 0 ? postLanguages[0].shortCode : ""
        root.composeReply(postUri, postCid, postText, postIndexedDateTime,
                          author, postReplyRootUri, postReplyRootCid, lang,
                          initialText, imageSource)
    }

    function rowsInsertedHandler(parent, start, end) {
        syncTimer.start()
    }

    Component.onDestruction: {
        view.model.onRowsInserted.disconnect(rowsInsertedHandler)
        skywalker.removePostThreadModel(modelId)
    }

    Component.onCompleted: {
        console.debug("Entry index:", postEntryIndex);
        view.model.onRowsInserted.connect(rowsInsertedHandler)

        // As not all entries have the same height, positioning at an index
        // is fickle. By moving to the end and then wait a bit before positioning
        // at the index entry, it seems to work.
        positionViewAtEnd()
        syncTimer.start()
    }
}
