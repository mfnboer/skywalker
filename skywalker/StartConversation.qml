import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

SkyPage {
    property var skywalker: root.getSkywalker()
    property bool isTyping: false

    signal closed
    signal selected(string did)

    id: page
    clip: true
    Material.background: guiSettings.backgroundColor

    header: SimpleHeader {
        text: qsTr("Start conversation")
        backIsCancel: true
        onBack: closed()

        Avatar {
            anchors.right: parent.right
            anchors.rightMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            height: parent.height - 10
            width: height
            author: skywalker.user
            onClicked: skywalker.showStatusMessage(qsTr("Yes, you're fabulous!"), QEnums.STATUS_LEVEL_INFO)
            onPressAndHold: skywalker.showStatusMessage(qsTr("Yes, you're really fabulous!"), QEnums.STATUS_LEVEL_INFO)

            Accessible.role: Accessible.Button
            Accessible.name: qsTr("your avatar")
            Accessible.onPressAction: clicked()
        }
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

    AccessibleText {
        anchors.centerIn: parent
        width: parent.width - 60
        font.italic: true
        wrapMode: Text.Wrap
        text: qsTr("Users that do not allow to be messaged are not shown in the search results.")
        visible: typeaheadView.count === 0
    }

    Timer {
        id: authorTypeaheadSearchTimer
        interval: 500
        onTriggered: {
            const text = searchInput.displayText

            if (text.length > 0)
                searchUtils.searchAuthorsTypeahead(text, 100, true)
            else
                resetAuthorTypeaheadList()
        }
    }

    SearchUtils {
        id: searchUtils
        skywalker: page.skywalker
    }

    GuiSettings {
        id: guiSettings
    }

    function resetAuthorTypeaheadList() {
        searchUtils.authorTypeaheadList = skywalker.chat.getAllConvoMembers()
    }

    Component.onCompleted: {
        resetAuthorTypeaheadList()
        searchInput.forceActiveFocus()
    }
}
