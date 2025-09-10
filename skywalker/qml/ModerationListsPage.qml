import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyPage {
    required property var skywalker
    required property int modelId
    readonly property string sideBarTitle: qsTr("Moderation lists")
    readonly property SvgImage sideBarSvg: SvgOutline.list

    id: page

    signal closed

    Accessible.role: Accessible.Pane

    header: SimpleHeader {
        text: sideBarTitle
        visible: !root.showSideBar
        onBack: page.closed()
    }

    SkyTabBar {
        id: listsBar
        y: !root.showSideBar ? 0 : guiSettings.headerMargin
        width: parent.width

        AccessibleTabButton {
            text: qsTr("Your lists")
        }
        AccessibleTabButton {
            text: qsTr("Blocked lists")
        }
        AccessibleTabButton {
            text: qsTr("Muted lists")
        }
    }

    SwipeView {
        anchors.top: listsBar.bottom
        anchors.bottom: parent.bottom
        width: parent.width
        currentIndex: listsBar.currentIndex

        onCurrentIndexChanged: {
            if (currentIndex === 1)
                blockedLists.refresh()
            else if (currentIndex === 2)
                mutedLists.refresh()

            listsBar.setCurrentIndex(currentIndex)
        }

        ListListView {
            id: yourLists
            skywalker: page.skywalker
            modelId: page.modelId
            ownLists: true
            description: qsTr("Public, shareable lists of users to mute or block in bulk.")
        }

        ListListView {
            id: blockedLists
            skywalker: page.skywalker
            modelId: skywalker.createListListModel(QEnums.LIST_TYPE_BLOCKS, QEnums.LIST_PURPOSE_MOD, "")
            ownLists: false
            description: qsTr("Lists blocked by you.")
        }

        ListListView {
            id: mutedLists
            skywalker: page.skywalker
            modelId: skywalker.createListListModel(QEnums.LIST_TYPE_MUTES, QEnums.LIST_PURPOSE_MOD, "")
            ownLists: false
            description: qsTr("Lists muted by you.")
        }
    }
}
