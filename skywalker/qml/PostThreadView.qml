import QtQuick
import QtQuick.Controls
import skywalker

SkyListView {
    property string userDid
    required property int modelId
    required property int postEntryIndex
    property int syncToIndex: postEntryIndex
    property Skywalker skywalker: root.getSkywalker(userDid)
    readonly property bool isUnrolledThread: model?.unrollThread
    readonly property string sideBarTitle: isUnrolledThread ? qsTr("Unrolled thread") : qsTr("Post thread")
    readonly property SvgImage sideBarSvg: isUnrolledThread ? SvgOutline.thread : SvgOutline.chat
    readonly property SvgImage sideBarButtonSvg: SvgOutline.moreVert
    readonly property string sideBarButtonName: qsTr("options")

    signal closed

    id: view
    width: parent.width
    model: skywalker.getPostThreadModel(modelId)
    cacheBuffer: Screen.height * 3
    boundsBehavior: Flickable.StopAtBounds

    header: SimpleHeader {
        height: (headerVisible ? guiSettings.headerHeight : 0) + (restrictionRow.visible ? restrictionRow.height : 0)
        text: sideBarTitle
        userDid: view.userDid
        headerVisible: !root.showSideBar
        onBack: view.closed()

        SvgPlainButton {
            anchors.top: parent.top
            anchors.right: parent.right

            id: replyOrderButton
            iconColor: guiSettings.headerTextColor
            // TODO temp icon. Replace with generic "sort" icon
            svg: SvgOutline.sortByAlpha
            accessibleName: qsTr("reply order")
            onClicked: orderMenuLoader.open()

            SkyMenuLoader {
                id: orderMenuLoader
                sourceComponent: orderMenuComponent
            }
        }

        Rectangle {
            id: restrictionRect
            width: parent.width
            height: restrictionRow.height + 5
            anchors.bottom: parent.bottom
            color: guiSettings.threadStartColor(skywalker.getUserSettings().threadColor)
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
                    y: height + 5
                    width: 20
                    height: 20
                    color: utils.determineForegroundColor(restrictionRect.color, "black", "white")
                    svg: SvgOutline.replyRestrictions
                }
                SkyCleanedText {
                    id: restrictionText
                    anchors.left: restrictionIcon.right
                    anchors.right: parent.right
                    topPadding: 5
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
                            root.viewListByUri(link, false, userDid)
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
        height: 0
        z: guiSettings.footerZLevel

        PostButton {
            y: -height - 10
            svg: SvgOutline.reply
            overrideOnClicked: () => reply()
            visible: !root.showSideBar && !isUnrolledThread

            Accessible.role: Accessible.Button
            Accessible.name: qsTr(`reply to ${(getReplyToAuthor()?.name)}`)
            Accessible.onPressAction: clicked()
        }
    }
    footerPositioning: ListView.OverlayFooter

    delegate: PostFeedViewDelegate {
        width: view.width
        unrollThread: isUnrolledThread
        postThreadModel: view.model

        onShowHiddenReplies: {
            syncToIndex = index
            model.showHiddenReplies()
        }
        onAddMorePosts: (uri) => {
            syncToIndex = index
            skywalker.addPostThread(uri, modelId)
        }
        onAddOlderPosts: {
            syncToIndex = 0
            skywalker.addOlderPostThread(modelId)
        }
    }

    FlickableRefresher {
        inProgress: skywalker.getPostThreadInProgress
    }

    Item {
        id: moreMenuItem
        anchors.right: parent.right
        SkyMenu {
            id: moreMenu

            CloseMenuItem {
                text: qsTr("<b>Options</b>")
                Accessible.name: qsTr("close options menu")
            }
            AccessibleMenuItem {
                text: qsTr("Settings")
                svg: SvgOutline.settings
                onTriggered: root.editPostThreadSettings(() => {
                    const userSettings = skywalker.getUserSettings()
                    const replyOrder = userSettings.getReplyOrder(userDid)
                    model.setReplyOrder(replyOrder)
                })
            }
        }
    }

    Component {
        id: orderMenuComponent

        SkyMenu {
            id: orderMenu

            ButtonGroup { id: radioGroup }

            CloseMenuItem {
                text: qsTr("<b>Sort Replies</b>")
                Accessible.name: qsTr("close sort replies menu")
            }

            SkyRadioMenuItem {
                text: qsTr("Smart")
                onTriggered: setReplySortOrder(QEnums.REPLY_ORDER_SMART)
                checkable: true
                ButtonGroup.group: radioGroup
                checked: model.getReplyOrder() === QEnums.REPLY_ORDER_SMART
            }

            SkyRadioMenuItem {
                text: qsTr("Oldest reply first")
                onTriggered: setReplySortOrder(QEnums.REPLY_ORDER_OLDEST_FIRST)
                checkable: true
                ButtonGroup.group: radioGroup
                checked: model.getReplyOrder() === QEnums.REPLY_ORDER_OLDEST_FIRST
            }

            SkyRadioMenuItem {
                text: qsTr("Newest reply first")
                onTriggered: setReplySortOrder(QEnums.REPLY_ORDER_NEWEST_FIRST)
                checkable: true
                ButtonGroup.group: radioGroup
                checked: model.getReplyOrder() === QEnums.REPLY_ORDER_NEWEST_FIRST
            }

            SkyRadioMenuItem {
                text: qsTr("Most popular first")
                onTriggered: setReplySortOrder(QEnums.REPLY_ORDER_POPULARITY)
                checkable: true
                ButtonGroup.group: radioGroup
                checked: model.getReplyOrder() === QEnums.REPLY_ORDER_POPULARITY
            }

            SkyRadioMenuItem {
                text: qsTr("Controversial first")
                onTriggered: setReplySortOrder(QEnums.REPLY_ORDER_CONTROVERSIAL)
                checkable: true
                ButtonGroup.group: radioGroup
                checked: model.getReplyOrder() === QEnums.REPLY_ORDER_CONTROVERSIAL
            }

            MenuSeparator {}

            AccessibleMenuItem {
                text: qsTr("Sort Thread First")
                onToggled: flipThreadFirst()
                checkable: true
                checked: model.getReplyOrderThreadFirst()
            }

            MenuSeparator {}

            AccessibleMenuItem {
                text: qsTr("Save As Default")
                onTriggered: saveReplySorting()
            }
        }
    }

    Utils {
        id: utils
        skywalker: view.skywalker
    }

    function sideBarButtonClicked() {
        moreMenu.open()
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
        const postMentionDids = model.getData(postEntryIndex, AbstractPostFeedModel.PostMentionDids)
        const postLanguages = model.getData(postEntryIndex, AbstractPostFeedModel.PostLanguages)

        const lang = postLanguages.length > 0 ? postLanguages[0].shortCode : ""
        root.composeReply(postUri, postCid, postText, postIndexedDateTime,
                          author, postReplyRootUri, postReplyRootCid, lang, postMentionDids,
                          initialText, imageSource, "", "", userDid)
    }

    function videoReply(initialText, videoSource) {
        const postUri = model.getData(postEntryIndex, AbstractPostFeedModel.PostUri)
        const postCid = model.getData(postEntryIndex, AbstractPostFeedModel.PostCid)
        const postText = model.getData(postEntryIndex, AbstractPostFeedModel.PostText)
        const postIndexedDateTime = model.getData(postEntryIndex, AbstractPostFeedModel.PostIndexedDateTime)
        const author = getReplyToAuthor()
        const postReplyRootUri = model.getData(postEntryIndex, AbstractPostFeedModel.PostReplyRootUri)
        const postReplyRootCid = model.getData(postEntryIndex, AbstractPostFeedModel.PostReplyRootCid)
        const postMentionDids = model.getData(postEntryIndex, AbstractPostFeedModel.PostMentionDids)
        const postLanguages = model.getData(postEntryIndex, AbstractPostFeedModel.PostLanguages)

        const lang = postLanguages.length > 0 ? postLanguages[0].shortCode : ""
        root.composeVideoReply(postUri, postCid, postText, postIndexedDateTime,
                               author, postReplyRootUri, postReplyRootCid, lang, postMentionDids,
                               initialText, videoSource, userDid)
    }

    function sync(index) {
        let firstVisibleIndex = getFirstVisibleIndex()
        let lastVisibleIndex = getLastVisibleIndex()
        console.debug("Move to:", index, "first:", firstVisibleIndex, "last:", lastVisibleIndex, "count:", count, "content:", contentHeight)
        positionViewAtIndex(index, ListView.Center)

        firstVisibleIndex = getFirstVisibleIndex()
        lastVisibleIndex = getLastVisibleIndex()
        return (firstVisibleIndex <= index && lastVisibleIndex >= index)
    }

    function rowsInsertedHandler(parent, start, end) {
        const inserted = end - start + 1
        console.debug("Rows inserted, start:", start, "end:", end, "count:", count, "inserted:", inserted)

        if (start === 0 && inserted !== count) {
            moveToIndex(end + 1, sync)
            syncToIndex += inserted
        }
        else {
            moveToIndex(syncToIndex, sync)
        }
    }

    function setReplySortOrder(replyOrder) {
        model.setReplyOrder(replyOrder)
    }

    function flipThreadFirst() {
        model.setReplyOrderThreadFirst(!model.getReplyOrderThreadFirst())
    }

    function saveReplySorting() {
        userSettings.setReplyOrder(userDid, model.getReplyOrder())
        userSettings.setReplyOrderThreadFirst(userDid, model.getReplyOrderThreadFirst())
    }

    Component.onDestruction: {
        model.onRowsInserted.disconnect(rowsInsertedHandler)
        skywalker.removePostThreadModel(modelId)
    }

    Component.onCompleted: {
        console.debug("Entry index:", postEntryIndex)
        model.onRowsInserted.connect(rowsInsertedHandler)

        if (model.unrollThread)
            positionViewAtBeginning()
        else
            moveToIndex(postEntryIndex, sync)
    }
}

