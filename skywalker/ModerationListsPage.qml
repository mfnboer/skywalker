import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Page {
    required property var skywalker
    required property int modelId

    id: page

    signal closed

    header: SimpleHeader {
        text: qsTr("Moderation lists")
        onBack: page.closed()
    }

    TabBar {
        id: listsBar
        width: parent.width

        TabButton {
            text: qsTr("Your lists")
        }
        TabButton {
            text: qsTr("Blocked lists")
        }
        TabButton {
            text: qsTr("Muted lists")
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
            description: qsTr("Public, shareable lists of users to mute or block in bulk.")
        }
    }

    GuiSettings {
        id: guiSettings
    }
}
