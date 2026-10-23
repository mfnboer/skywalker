import QtQuick
import skywalker

Flow {
    property string userDid
    property Skywalker skywalker: root.getSkywalker(userDid)
    required property detailedprofile author

    spacing: 5

    Repeater {
        model: filterLinksToShow() // list<link, visibility>

        Rectangle {
            required property var modelData
            readonly property profilelinkview linkView: modelData[0]
            readonly property int visibility: modelData[1] // QEnums::ContentVisibility

            id: linkRect
            width: link.width + (linkIcon.active ? link.height + 2 : 0) + 10
            height: link.height + 4
            radius: height / 2
            color: guiSettings.linkButtonColor

            Loader {
                id: linkIcon
                x: 5
                y: 4
                active: linkRect.linkView.icon && linkRect.visibility === QEnums.CONTENT_VISIBILITY_SHOW
                asynchronous: true

                sourceComponent: RoundCornerMask {
                    width: height
                    height: link.height - 4
                    cornerRadius: width * 0.1
                    maskColor: linkRect.color

                    ImageAutoRetry {
                        id: iconImg
                        anchors.fill: parent
                        sourceSize.width: width * Screen.devicePixelRatio
                        sourceSize.height: height * Screen.devicePixelRatio
                        source: linkRect.linkView.icon
                        fillMode: Image.PreserveAspectFit
                        indicateLoading: false
                    }
                }
            }

            SkyLabel {
                id: link
                anchors.left: linkIcon.right
                anchors.leftMargin: linkIcon.active ? 2 : 0
                y: 2
                width: Math.min(implicitWidth, guiSettings.maxLinkButtonWidth)
                backgroundColor: "transparent"
                font.pointSize: guiSettings.linkButtonFontSize
                font.weight: Font.Medium
                elide: Text.ElideRight
                text: getDisplayIconPrefix() + getDisplayText()
            }

            SkyMouseArea {
                anchors.fill: parent
                onClicked: {
                    const warning = getWarning(linkView, visibility)
                    root.openLink(linkView.url, getDisplayText(), userDid, warning)
                }
            }

            function getDisplayText() {
                return linkView.title ? linkView.title : linkView.url
            }

            function getDisplayIconPrefix() {
                if (visibility === QEnums.CONTENT_VISIBILITY_SHOW)
                    return linkView.icon ? "" : "🌐 "

                return "⚠️ "
            }
        }
    }

    function filterLinksToShow() {
        let linksToSow = []
        const contentFilter = skywalker.getContentFilter()

        for (const link of author.links) {
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
