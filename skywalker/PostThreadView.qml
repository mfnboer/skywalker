import QtQuick
import skywalker

SkyListView {
    required property int modelId
    required property int postEntryIndex
    property bool syncToEntry: true
    property var skywalker: root.getSkywalker()

    signal closed

    id: view
    width: parent.width
    model: skywalker.getPostThreadModel(modelId)

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
            visible: model.replyRestriction !== QEnums.REPLY_RESTRICTION_NONE

            Accessible.role: Accessible.StaticText
            Accessible.name: UnicodeFonts.toPlainText(restrictionText.text)

            Rectangle {
                id: restrictionRow
                x: guiSettings.threadColumnWidth - restrictionIcon.width
                anchors.bottom: parent.bottom
                width: parent.width - x - 10
                height: restrictionText.height + 10
                color: "transparent"

                SkySvg {
                    id: restrictionIcon
                    width: 20
                    height: 20
                    color: utils.determineForegroundColor(restrictionRect.color, "black", "white")
                    svg: SvgOutline.replyRestrictions
                }
                SkyCleanedText {
                    id: restrictionText
                    anchors.left: restrictionIcon.right
                    anchors.right: parent.right
                    leftPadding: 5
                    color: restrictionIcon.color
                    ellipsisBackgroundColor: restrictionRect.color.toString()
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

                    if (replyRestriction & QEnums.REPLY_RESTRICTION_FOLLOWER) {
                        const author = model.replyRestrictionAuthor
                        restrictionList.push(qsTr(`users following <a href="${author.did}" style="color: ${guiSettings.linkColor}; text-decoration: none">@${author.handle}</a>`))
                    }

                    if (replyRestriction & QEnums.REPLY_RESTRICTION_FOLLOWING) {
                        const author = model.replyRestrictionAuthor
                        restrictionList.push(qsTr(`users followed by <a href="${author.did}" style="color: ${guiSettings.linkColor}; text-decoration: none">@${author.handle}</a>`))
                    }

                    if (replyRestriction & QEnums.REPLY_RESTRICTION_LIST) {
                        const lists = model.replyRestrictionLists
                        let listNames = []

                        for (let i = 0; i < lists.length; ++i) {
                            const l = lists[i]
                            const listName = UnicodeFonts.toCleanedHtml(l.name)
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
            svg: SvgOutline.reply
            overrideOnClicked: () => reply()

            Accessible.role: Accessible.Button
            Accessible.name: qsTr(`reply to ${(getReplyToAuthor().name)}`)
            Accessible.onPressAction: clicked()
        }
    }
    footerPositioning: ListView.OverlayFooter

    delegate: PostFeedViewDelegate {
        width: view.width
        onShowHiddenReplies: model.showHiddenReplies()
        onAddMorePosts: (uri) => skywalker.addPostThread(uri, modelId)
    }

    FlickableRefresher {
        inProgress: skywalker.getPostThreadInProgress
        topOvershootFun: () => {
            syncToEntry = false
            skywalker.getPostThread(model.getThreadEntryUri(), modelId)
        }
        topText: qsTr("Pull down to refresh")
    }


    Utils {
        id: utils
        skywalker: view.skywalker
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

    function videoReply(initialText, videoSource) {
        const postUri = model.getData(postEntryIndex, AbstractPostFeedModel.PostUri)
        const postCid = model.getData(postEntryIndex, AbstractPostFeedModel.PostCid)
        const postText = model.getData(postEntryIndex, AbstractPostFeedModel.PostText)
        const postIndexedDateTime = model.getData(postEntryIndex, AbstractPostFeedModel.PostIndexedDateTime)
        const author = getReplyToAuthor()
        const postReplyRootUri = model.getData(postEntryIndex, AbstractPostFeedModel.PostReplyRootUri)
        const postReplyRootCid = model.getData(postEntryIndex, AbstractPostFeedModel.PostReplyRootCid)
        const postLanguages = model.getData(postEntryIndex, AbstractPostFeedModel.PostLanguages)

        const lang = postLanguages.length > 0 ? postLanguages[0].shortCode : ""
        root.composeVideoReply(postUri, postCid, postText, postIndexedDateTime,
                               author, postReplyRootUri, postReplyRootCid, lang,
                               initialText, videoSource)
    }

    function sync() {
        const firstVisibleIndex = getFirstVisibleIndex()
        const lastVisibleIndex = getLastVisibleIndex()
        console.debug("Move to:", postEntryIndex, "first:", firstVisibleIndex, "last:", lastVisibleIndex, "count:", count, "content:", contentHeight)
        positionViewAtIndex(postEntryIndex, ListView.End)
        return (lastVisibleIndex >= postEntryIndex - 1 && lastVisibleIndex <= postEntryIndex + 1)
    }

    Component.onDestruction: {
        skywalker.removePostThreadModel(modelId)
    }

    Component.onCompleted: {
        console.debug("Entry index:", postEntryIndex);
        moveToIndex(postEntryIndex, sync)
    }
}
