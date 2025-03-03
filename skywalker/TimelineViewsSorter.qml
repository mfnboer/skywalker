import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import skywalker

SkyPage {
    required property var timelineModel

    signal closed

    id: page
    width: parent.width
    height: parent.height

    header: SimpleDescriptionHeader {
        title: qsTr("Sort timeline views")
        description: qsTr("To change the order, keep a view pushed till its background changes color, then drag it to the desired position.")
        onClosed: page.closed()
    }

    SkyListView {
        id: listView
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        spacing: 0
        boundsBehavior: Flickable.StopAtBounds

        model: DelegateModel {
            id: visualModel

            model: timelineModel.filteredPostFeedModels

            delegate: DragDropRectangle {
                id: rect
                width: listView.width
                height: row.height + 10
                color: guiSettings.backgroundColor

                Row {
                    id: row

                    anchors.centerIn: parent
                    width: parent.width - 20
                    spacing: 5

                    Avatar {
                        id: avatar
                        anchors.verticalCenter: parent.verticalCenter
                        width: visible ? parent.height : 0
                        author: modelData.profile
                        visible: !modelData.profile.isNull()
                    }

                    AccessibleText {
                        anchors.verticalCenter: parent.verticalCenter
                        color: guiSettings.textColor
                        text: modelData.feedName
                    }
                }
            }
        }
    }

    function reorderTimelineViews() {
        let models = []

        for (let i = 0; i < visualModel.items.count; ++i) {
            const item = visualModel.items.get(i)
            models.push(item.model.modelData)
        }

        timelineModel.reorderFilteredPostFeedModels(models)
    }

    Component.onDestruction: {
        reorderTimelineViews()
    }
}
