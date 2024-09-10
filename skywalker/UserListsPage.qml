import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyPage {
    required property var skywalker
    required property int modelId

    id: page

    signal closed

    Accessible.role: Accessible.Pane

    header: SimpleHeader {
        text: qsTr("User lists")
        onBack: page.closed()
    }

    TabBar {
        id: listsBar
        width: parent.width

        AccessibleTabButton {
            text: qsTr("Your lists")
        }
        AccessibleTabButton {
            text: qsTr("Saved lists")
        }
    }

    StackLayout {
        anchors.top: listsBar.bottom
        anchors.bottom: parent.bottom
        width: parent.width
        currentIndex: listsBar.currentIndex

        ListListView {
            id: yourLists
            width: parent.width
            height: parent.height
            skywalker: page.skywalker
            modelId: page.modelId
            ownLists: true
            description: qsTr("Public, shareable lists of users which can be used as feeds or reply restrictions.")
        }

        ListView {
            id: savedListsView
            width: parent.width
            height: parent.height
            spacing: 0
            clip: true
            model: skywalker.favoriteFeeds.getSavedListsModel()
            boundsBehavior: Flickable.StopAtBounds
            flickDeceleration: guiSettings.flickDeceleration
            maximumFlickVelocity: guiSettings.maxFlickVelocity
            pixelAligned: guiSettings.flickPixelAligned
            ScrollIndicator.vertical: ScrollIndicator {}

            header: Rectangle {
                width: parent.width
                height: headerText.height
                z: guiSettings.headerZLevel
                color: guiSettings.backgroundColor

                Text {
                    id: headerText
                    width: parent.width
                    padding: 10
                    wrapMode: Text.Wrap
                    elide: Text.ElideRight
                    color: guiSettings.textColor
                    text: qsTr("Saved lists from other users")
                }
            }
            headerPositioning: ListView.OverlayHeader

            delegate: ListViewDelegate {
                width: page.width
                ownLists: false
            }

            FlickableRefresher {}

            EmptyListIndication {
                y: parent.headerItem ? parent.headerItem.height : 0
                svg: svgOutline.noPosts
                text: qsTr("No saved lists")
                list: savedListsView
            }

            BusyIndicator {
                anchors.centerIn: parent
                running: skywalker.favoriteFeeds.updateSavedFeedsModelInProgress
            }
        }
    }

    GuiSettings {
        id: guiSettings
    }

    Component.onDestruction: {
        skywalker.favoriteFeeds.removeSavedListsModel()
    }
}
