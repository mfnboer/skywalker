import QtQuick
import skywalker

SvgButton {
    required property bool messagesActive
    required property bool hashtagSearch
    property var searchView
    property var authorView
    property var postThreadView

    signal addConvoClicked()

    topInset: 0
    leftInset: 0
    rightInset: 0
    bottomInset: 0
    svg: getSvg()
    accessibleName: qsTr("create post")

    onClicked: {
        if (messagesActive)
            addConvoClicked()
        else
            post()
    }

    function getSvg() {
        if (authorView)
            return authorView.authorCanBeMentioned() ? SvgOutline.atSign : SvgOutline.chat

        if (postThreadView)
            return SvgOutline.reply

        if (messagesActive)
            return SvgOutline.add

        if (hashtagSearch)
            return SvgOutline.hashtag

        return SvgOutline.chat
    }

    function post() {
        if (authorView) {
            if (authorView.authorCanBeMentioned())
                authorView.mentionPost()
            else
                root.composePost()
        }
        else if (postThreadView) {
            postThreadView.reply()
        }
        else if (hashtagSearch && searchView) {
            root.composePost("\n" + searchView.getSearchText())
        }
        else {
            root.composePost()
        }
    }
}
