import QtQuick
import skywalker

SkyListView {
    required property list<profilelinkview> links
    property Skywalker skywalker: root.getSkywalker()

    signal clicked(int index)
    signal orderChanged

    id: listView
    boundsBehavior: Flickable.StopAtBounds

    model: DelegateModel {
        id: visualModel

        model: links

        delegate: DragDropRectangle {
            id: rect
            width: listView.width
            height: linkButton.height + 10
            onClicked: listView.clicked(visualIndex)
            onDragged: listView.orderChanged()

            Row {
                anchors.verticalCenter: parent.verticalCenter

                SkySvg {
                    width: height
                    height: linkButton.height
                    svg: SvgOutline.dragIndicator
                }

                ProfileLinkButton {
                    id: linkButton
                    linkView: rect.modelData
                    enabled: false
                    visibility: QEnums.CONTENT_VISIBILITY_SHOW
                }
            }
        }
    }

    LinkUtils {
        id: linkUtils
        skywalker: listView.skywalker
    }

    function getLinks() {
        let links = []

        for (let i = 0; i < visualModel.items.count; ++i) {
            const item = visualModel.items.get(i)
            links.push(item.model.modelData)
        }

        return links
    }

    function containsLink(url, skipIndex = -1) {
        const links = getLinks()

        for (let i = 0; i < links.length; ++i) {
            if (i === skipIndex)
                continue

            const link = links[i]

            if (linkUtils.equalLinks(url, link.url))
                return true
        }

        return false
    }
}
