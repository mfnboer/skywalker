import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Page {
    required property var skywalker
    required property int modelId

    id: page
    Material.background: guiSettings.backgroundColor

    signal closed

    Accessible.role: Accessible.Pane

    header: SimpleHeader {
        text: qsTr("Moderation lists")
        onBack: page.closed()
    }

    TabBar {
        id: listsBar
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

    StackLayout {
        anchors.top: listsBar.bottom
        anchors.bottom: parent.bottom
        width: parent.width
        currentIndex: listsBar.currentIndex

        onCurrentIndexChanged: {
            if (currentIndex === 1)
                blockedLists.refresh()
            else if (currentIndex === 2)
                mutedLists.refresh()
        }

        ListListView {
            id: yourLists
            width: parent.width
            height: parent.height
            skywalker: page.skywalker
            modelId: page.modelId
            ownLists: true
            description: qsTr("Public, shareable lists of users to mute or block in bulk.")
        }

        ListListView {
            id: blockedLists
            width: parent.width
            height: parent.height
            skywalker: page.skywalker
            modelId: skywalker.createListListModel(QEnums.LIST_TYPE_BLOCKS, QEnums.LIST_PURPOSE_MOD, "")
            ownLists: false
            description: qsTr("Lists blocked by you.")
        }

        ListListView {
            id: mutedLists
            width: parent.width
            height: parent.height
            skywalker: page.skywalker
            modelId: skywalker.createListListModel(QEnums.LIST_TYPE_MUTES, QEnums.LIST_PURPOSE_MOD, "")
            ownLists: false
            description: qsTr("Lists muted by you.")
        }
    }

    GuiSettings {
        id: guiSettings
    }
}
