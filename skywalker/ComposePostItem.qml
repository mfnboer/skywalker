import QtQuick
import skywalker

Item {
    property string text: ""
    property list<string> images: []
    property list<string> altTexts: []
    property list<string> memeTopTexts: []
    property list<string> memeBottomTexts: []
    property basicprofile quoteAuthor
    property string quoteUri: ""
    property string quoteCid: ""
    property string quoteText: ""
    property date quoteDateTime: new Date()
    property bool quoteFixed: false
    property generatorview quoteFeed
    property listview quoteList
    property tenorgif gif
    property var card: null
    property string language: ""
    property string video: ""
    property string videoAltText: ""

    // Content warnings
    property bool cwSuggestive: false
    property bool cwNudity: false
    property bool cwPorn: false
    property bool cwGore: false

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

    function hasImageContent() {
        return gif || (card && card.thumb) || images.length > 0
    }

    function imageHasMeme(index) {
        return index < images.length && (memeTopTexts[index].length > 0 || memeBottomTexts[index].length > 0)
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
            labels.push("graphic-media")

        return labels
    }

    function setContentWarnings(labels) {
        cwSuggestive = false
        cwNudity = false
        cwPorn = false
        cwGore = false

        labels.forEach((label) => {
            if (label === "sexual")
                cwSuggestive = true
            else if (label === "nudity")
                cwNudity = true
            else if (label === "porn")
                cwPorn = true
            else if (label === "gore" || label === "graphic-media")
                cwGore = true
        })
    }
}
