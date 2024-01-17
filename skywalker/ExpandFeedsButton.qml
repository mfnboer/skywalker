import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SvgButton {
    required property var skywalker

    iconColor: guiSettings.headerTextColor
    Material.background: "transparent"
    svg: svgOutline.expandMore
    onClicked: feedsMenu.open()

    Menu {
        id: feedsMenu

        MenuItem {
            text: qsTr("Home feed", "timeline title")

            FeedAvatar {
                y: 5
                anchors.rightMargin: 10
                anchors.right: parent.right
                width: height
                height: parent.height - 10
                unknownSvg: svgFilled.home
                onClicked: parent.triggered()
            }

            onTriggered: root.viewTimeline()
        }

        Instantiator {
            id: menuInstantiator
            model: []
            delegate: MenuItem {
                text: modelData.name

                FeedAvatar {
                    y: 5
                    anchors.rightMargin: 10
                    anchors.right: parent.right
                    width: height
                    height: parent.height - 10
                    avatarUrl: modelData.avatar
                    unknownSvg: modelData.isGeneratorView ? svgFilled.feed : svgFilled.list
                    onClicked: parent.triggered()
                }

                onTriggered: viewFeed()

                function viewFeed() {
                    if (modelData.isGeneratorView)
                        root.viewFeed(modelData.generatorView)
                    else
                        root.viewListFeed(modelData.listView)
                }
            }

            onObjectAdded: (index, object) => {
                console.debug("Add feed menu item:", object.text, "index:", index)
                feedsMenu.insertItem(index + 1, object)
            }
            onObjectRemoved: (index, object) => {
                console.debug("Remove feed menu item:", object.text, "index:", index)
                feedsMenu.removeItem(object)
            }
        }

        onAboutToShow: menuInstantiator.model = skywalker.favoriteFeeds.getPinnedFeeds()
        onAboutToHide: menuInstantiator.model = []
    }

    GuiSettings {
        id: guiSettings
    }
}
