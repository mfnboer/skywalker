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

            ProfileLinkButton {
                id: linkButton
                anchors.verticalCenter: parent.verticalCenter
                linkView: rect.modelData
                enabled: false
                visibility: QEnums.CONTENT_VISIBILITY_SHOW
            }
        }
    }

    function getLinks() {
        let links = []

        for (let i = 0; i < visualModel.items.count; ++i) {
            const item = visualModel.items.get(i)
            links.push(item.model.modelData)
        }

        return links
    }
}
