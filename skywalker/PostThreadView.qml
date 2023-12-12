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
    ScrollIndicator.vertical: ScrollIndicator {}

    header: SimpleHeader {
        height: restrictionRow.visible ? guiSettings.headerHeight + restrictionRow.height : guiSettings.headerHeight
        text: qsTr("Post thread")
        onBack: view.closed()

        Rectangle {
            width: parent.width
            height: restrictionRow.height + 5
            anchors.bottom: parent.bottom
            color: guiSettings.threadStartColor
            border.width: 1
            border.color: guiSettings.headerColor
            visible: model.getReplyRestriction() !== QEnums.REPLY_RESTRICTION_NONE

            Rectangle {
                id: restrictionRow
                x: guiSettings.threadBarWidth * 5
                anchors.bottom: parent.bottom
                width: parent.width - x - 10
                height: restrictionText.height + 5
                color: "transparent"

                SvgImage {
                    id: restrictionIcon
                    width: restrictionText.height
                    height: restrictionText.height
                    color: guiSettings.textColor
                    svg: svgOutline.replyRestrictions
                }
                Text {
                    id: restrictionText
                    anchors.left: restrictionIcon.right
                    anchors.right: parent.right
                    leftPadding: 5
                    color: guiSettings.textColor
                    font.italic: true
                    font.pointSize: guiSettings.scaledFont(7/8)
                    wrapMode: Text.Wrap
                    text: restrictionRow.getRestrictionText()
                }

                function getRestrictionText() {
                    const replyRestriction = model.getReplyRestriction()

                    if (replyRestriction === QEnums.REPLY_RESTRICTION_NONE)
                        return ""

                    if (replyRestriction === QEnums.REPLY_RESTRICTION_NOBODY)
                        return qsTr("Replies are disabled")

                    let restrictionList = []

                    if (replyRestriction & QEnums.REPLY_RESTRICTION_MENTIONED)
                        restrictionList.push(qsTr("mentioned"))
                    if (replyRestriction & QEnums.REPLY_RESTRICTION_FOLLOWING)
                        restrictionList.push(qsTr("followed"))
                    if (replyRestriction & QEnums.REPLY_RESTRICTION_LIST)
                        restrictionList.push(qsTr("selected"))

                    if (!restrictionList) {
                        console.warn("No restrictions found.")
                        return qsTr("Replies are restricted")
                    }

                    let restrictionListText = restrictionList[0]

                    for (let i = 1; i < restrictionList.length - 1; ++i)
                        restrictionListText += ", " + restrictionList[i]

                    if (restrictionList.length > 1) {
                        restrictionListText += " and "
                        restrictionListText += restrictionList[restrictionList.length - 1]
                    }

                    return qsTr(`Replies are restricted to ${restrictionListText} users`)
                }
            }
        }
    }
    headerPositioning: ListView.OverlayHeader


    footer: Rectangle {
        width: parent.width
        z: guiSettings.footerZLevel

        PostButton {
            x: parent.width - width - 10
            y: -height - 10
            svg: svgOutline.reply
            overrideOnClicked: () => reply()
        }
    }
    footerPositioning: ListView.OverlayFooter

    delegate: PostFeedViewDelegate {
        viewWidth: view.width
    }

    Timer {
        id: syncTimer
        interval: 100
        onTriggered: positionViewAtIndex(postEntryIndex, ListView.Center)
    }

    GuiSettings {
        id: guiSettings
    }

    function reply(initialText = "", imageSource = "") {
        const postUri = model.getData(postEntryIndex, AbstractPostFeedModel.PostUri)
        const postCid = model.getData(postEntryIndex, AbstractPostFeedModel.PostCid)
        const postText = model.getData(postEntryIndex, AbstractPostFeedModel.PostText)
        const postIndexedDateTime = model.getData(postEntryIndex, AbstractPostFeedModel.PostIndexedDateTime)
        const author = model.getData(postEntryIndex, AbstractPostFeedModel.Author)
        const postReplyRootUri = model.getData(postEntryIndex, AbstractPostFeedModel.PostReplyRootUri)
        const postReplyRootCid = model.getData(postEntryIndex, AbstractPostFeedModel.PostReplyRootCid)

        root.composeReply(postUri, postCid, postText, postIndexedDateTime,
                          author, postReplyRootUri, postReplyRootCid,
                          initialText, imageSource)
    }

    Component.onCompleted: {
        console.debug("Entry index:", postEntryIndex);

        // As not all entries have the same height, positioning at an index
        // is fickle. By moving to the end and then wait a bit before positioning
        // at the index entry, it seems to work.
        positionViewAtEnd()
        syncTimer.start()
    }
    Component.onDestruction: skywalker.removePostThreadModel(modelId)
}
