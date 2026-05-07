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

    SkyTextInput {
        id: searchInput
        width: parent.width
        svgIcon: SvgOutline.user
        placeholderText: qsTr("Search user")
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
        canChatOnly: true

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
        const convoMembers = skywalker.chat.getAllAcceptedConvoMembers()
        typeaheadView.reset(convoMembers)
    }

    Component.onCompleted: {
        resetAuthorTypeaheadList()
        searchInput.forceActiveFocus()
    }
}
