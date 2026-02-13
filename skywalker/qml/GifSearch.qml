import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyPage {
    readonly property string sideBarTitle: qsTr("Add GIF")
    readonly property SvgImage sideBarSvg: SvgOutline.addGif

    signal closed
    signal selected(tenorgif gif)

    id: page

    SkyTabBar {
        id: tabBar
        width: parent.width
        clip: true

        AccessibleTabButton {
            text: qsTr("Tenor")
        }
        AccessibleTabButton {
            text: qsTr("Giphy")
        }
    }

    SwipeView {
        id: swipeView
        anchors.top: tabBar.bottom
        anchors.bottom: parent.bottom
        width: parent.width
        currentIndex: tabBar.currentIndex

        onCurrentIndexChanged: tabBar.setCurrentIndex(currentIndex)

        TenorSearch {
            onClosed: page.closed()
            onSelected: (gif) => page.selected(gif)
        }

        GiphySearch {
            onClosed: page.closed()
            onSelected: (gif) => page.selected(gif)
        }
    }

    function init() {
        // HACK: After reopening the page shows tab 0 even if currentIndex=1.
        // Toggling to 0 and then to currentIndex makes sure the correct tab shows.
        const index = swipeView.currentIndex
        swipeView.setCurrentIndex(0)
        swipeView.setCurrentIndex(index)
    }

    function cancel() {
        swipeView.currentItem.cancel()
    }
}
