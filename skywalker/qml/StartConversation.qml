import QtQuick
import skywalker

SkyPage {
    property var skywalker: root.getSkywalker()
    property bool isTyping: false
    readonly property string sideBarTitle: qsTr("Start conversation")
    readonly property SvgImage sideBarSvg: SvgOutline.directMessage

    signal closed
    signal selected(string did)

    id: page
    padding: 10
    clip: true

    header: SimpleHeader {
        text: sideBarTitle
        backIsCancel: true
        visible: !root.showSideBar
        onBack: closed()

        Avatar {
            anchors.right: parent.right
            anchors.rightMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            width: parent.height - 10
            author: skywalker.user
            onClicked: skywalker.showStatusMessage(qsTr("Yes, you're fabulous!"), QEnums.STATUS_LEVEL_INFO)
            onPressAndHold: skywalker.showStatusMessage(qsTr("Yes, you're really fabulous!"), QEnums.STATUS_LEVEL_INFO)

            Accessible.role: Accessible.Button
            Accessible.name: qsTr("your avatar")
            Accessible.onPressAction: clicked()
        }
    }

    AccessibleText {
        id: groupLink
        anchors.horizontalCenter: parent.horizontalCenter
        text: qsTr(`<a href="settings" style="color: ${guiSettings.linkColor}">New group chat</a>`)
        textFormat: Text.RichText
        onLinkActivated: newGroupChat()
    }

    SkyTextInput {
        id: searchInput
        anchors.top: groupLink.bottom
        anchors.topMargin: 15
        width: parent.width
        svgIcon: SvgOutline.user
        placeholderText: qsTr("Search user for a 1-1 chat")
        inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText

        onDisplayTextChanged: {
            page.isTyping = true
            typeaheadView.startSearch()
        }

        onEditingFinished: {
            page.isTyping = false
            typeaheadView.stopSearch()
        }
    }

    SimpleAuthorTypeaheadListView {
        id: typeaheadView
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: searchInput.bottom
        anchors.bottom: parent.bottom
        searchText: searchInput.displayText
        searchLimit: 100
        searchFilter: QEnums.AUTHOR_SEARCH_FILTER_CHAT_ONLY

        onAuthorClicked: (profile) => selected(profile.did)
        onCleared: resetAuthorTypeaheadList()
    }

    AccessibleText {
        anchors.centerIn: parent
        width: parent.width - 60
        font.italic: true
        wrapMode: Text.Wrap
        text: qsTr("Users that do not allow to be messaged are not shown in the search results.")
        visible: typeaheadView.count === 0
    }

    function resetAuthorTypeaheadList() {
        const convoMembers = skywalker.chat.getAcceptedConvoMembers()
        typeaheadView.reset(convoMembers)
    }

    function newGroupChat() {
        let component = guiSettings.createComponent("NewGroupChatDialog.qml")
        let dialog = component.createObject(page)

        dialog.onAccepted.connect(() => {
            const name = dialog.getText()

            if (name)
                skywalker.chat.createGroupConvo(name)

            dialog.close()
            page.closed()
        })

        dialog.onRejected.connect(() => dialog.close())
        dialog.open()
    }

    Component.onCompleted: {
        resetAuthorTypeaheadList()
        searchInput.forceActiveFocus()
    }
}
