import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Page {
    property var skywalker: root.getSkywalker()
    property bool isTyping: false

    signal closed
    signal selected(string did)

    id: page
    clip: true

    header: SimpleHeader {
        text: qsTr("Start conversation")
        backIsCancel: true
        onBack: closed()
    }

    SkyTextInput {
        id: searchInput
        width: parent.width
        svgIcon: svgOutline.user
        placeholderText: qsTr("Search user")

        onDisplayTextChanged: {
            page.isTyping = true
            authorTypeaheadSearchTimer.start()
        }

        onEditingFinished: {
            page.isTyping = false
            authorTypeaheadSearchTimer.stop()
        }
    }

    SimpleAuthorListView {
        id: typeaheadView
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: searchInput.bottom
        anchors.bottom: parent.bottom
        model: searchUtils.authorTypeaheadList

        onAuthorClicked: (profile) => selected(profile.did)
    }

    Timer {
        id: authorTypeaheadSearchTimer
        interval: 500
        onTriggered: {
            const text = searchInput.displayText

            if (text.length > 0)
                searchUtils.searchAuthorsTypeahead(text, 100, true)
        }
    }

    SearchUtils {
        id: searchUtils
        skywalker: page.skywalker
    }

    GuiSettings {
        id: guiSettings
    }

    Component.onCompleted: {
        searchInput.forceActiveFocus()
    }
}
