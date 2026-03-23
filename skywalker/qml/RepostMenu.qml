import QtQuick
import skywalker

SkyMenu {
    property string repostByDid
    property string repostedAlreadyUri
    property string repostUri
    property string repostCid
    property string repostViaUri
    property string repostViaCid
    property string repostFeedDid
    property string repostFeedContext
    property string repostText
    property date repostDateTime
    property basicprofile repostAuthor
    property bool repostEmbeddingDisabled
    property string repostPlainText

    id: repostMenu
    menuWidth: 240

    SkyMenuButton {
        svg: SvgOutline.repost
        popup: repostMenu
        text: repostMenu.repostedAlreadyUri ? qsTr("Undo repost") : qsTr("Repost")

        onClicked: {
            const pu = root.getPostUtils(repostMenu.repostByDid)

            if (repostMenu.repostedAlreadyUri) {
                pu.undoRepost(repostMenu.repostedAlreadyUri, repostMenu.repostUri,
                              repostMenu.repostCid, repostMenu.repostFeedDid)
            } else {
                pu.repost(repostMenu.repostUri, repostMenu.repostCid,
                          repostMenu.repostViaUri, repostMenu.repostViaCid,
                          repostMenu.repostFeedDid, repostMenu.repostFeedContext)
            }
        }
    }

    SkyMenuButton {
        svg: SvgOutline.quote
        popup: repostMenu
        text: qsTr("Quote post")
        enabled: !repostMenu.repostEmbeddingDisabled

        onClicked: {
            // No need to check if post still exist. Already checked before
            // opening this drawer
            root.doComposeQuote(repostMenu.repostUri, repostMenu.repostCid,
                                repostMenu.repostText, repostMenu.repostDateTime,
                                repostMenu.repostAuthor, "",
                                repostMenu.repostFeedDid, repostMenu.repostFeedContext,
                                repostMenu.repostByDid)
        }
    }

    SkyMenuButton {
        svg: SvgOutline.copy
        text: qsTr("Copy & quote post")
        popup: repostMenu
        enabled: !repostMenu.repostEmbeddingDisabled
        onClicked: {
            // No need to check if post still exist. Already checked before
            // opening this drawer
            root.doComposeQuote(repostMenu.repostUri, repostMenu.repostCid,
                                repostMenu.repostText, repostMenu.repostDateTime,
                                repostMenu.repostAuthor, repostMenu.repostPlainText,
                                repostMenu.repostFeedDid, repostMenu.repostFeedContext,
                                repostMenu.repostByDid)
        }
    }

    SkyMenuButton {
        svg: SvgOutline.chat
        text: qsTr("Quote in direct message")
        popup: repostMenu
        enabled: !repostMenu.repostEmbeddingDisabled
        visible: root.isActiveUser(repostMenu.repostByDid)

        onClicked: {
            const lu = root.getLinkUtils(repostMenu.repostByDid)
            const link = lu.toHttpsLink(repostMenu.repostUri)
            root.startConvo(link)
        }
    }

    function show(hasRepostedUri, uri, cid, viaUri, viaCid, feedDid, feedContext, text,
                  dateTime, author, embeddingDisabled, plainText, byDid = "") {
        repostByDid = byDid
        repostedAlreadyUri =  hasRepostedUri
        repostUri = uri
        repostCid = cid
        repostViaUri = viaUri
        repostViaCid = viaCid
        repostFeedDid = feedDid
        repostFeedContext = feedContext
        repostText = text
        repostDateTime = dateTime
        repostAuthor = author
        repostEmbeddingDisabled = embeddingDisabled
        repostPlainText = plainText

        open()
    }
}
