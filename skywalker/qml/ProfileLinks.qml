import QtQuick
import skywalker

Flow {
    property string userDid
    property Skywalker skywalker: root.getSkywalker(userDid)
    required property detailedprofile author
    required property list<profilelinkview> links

    spacing: 5

    Repeater {
        model: filterLinksToShow() // list<link, visibility>

        ProfileLinkButton {
            required property var modelData

            linkView: modelData[0]
            visibility: modelData[1] // QEnums::ContentVisibility

            onClicked: {
                const warning = getWarning(linkView, visibility)
                root.openLink(linkView.url, getDisplayText(), userDid, warning)
            }
        }
    }

    function filterLinksToShow() {
        let linksToSow = []
        const contentFilter = skywalker.getContentFilter()

        for (const link of links) {
            const visibility = link.getContentVisibility(contentFilter, author)
            if (visibility < QEnums.CONTENT_VISIBILITY_HIDE_MEDIA)
                linksToSow.push([link, visibility])
        }

        return linksToSow
    }

    function getWarning(linkView, visibility) {
        if (visibility === QEnums.CONTENT_VISIBILITY_SHOW)
            return ""

        const contentFilter = skywalker.getContentFilter()
        return linkView.getContentVisibility(contentFilter, author)
    }
}
