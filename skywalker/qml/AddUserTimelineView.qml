import QtQuick
import skywalker

SkyPage {
    property var skywalker: root.getSkywalker()
    readonly property string sideBarTitle: qsTr("Add user view")
    readonly property SvgImage sideBarSvg: SvgOutline.user


    signal closed
    signal selected(basicprofile profile)

    id: page
    padding: 10
    clip: true

    header: SimpleHeader {
        text: sideBarTitle
        backIsCancel: true
        visible: !root.showSideBar
        onBack: closed()
    }

    SkyTextInput {
        id: searchInput
        width: parent.width
        svgIcon: SvgOutline.user
        placeholderText: qsTr("Search user")
        inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText

        onDisplayTextChanged: {
            typeaheadView.startSearch()
        }

        onEditingFinished: {
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

        onAuthorClicked: (profile) => selected(profile)
    }

    AccessibleText {
        anchors.top: searchInput.bottom
        topPadding: 10
        width: parent.width - 20
        font.italic: true
        wrapMode: Text.Wrap
        text: qsTr("A user view shows posts from your timeline posted by a specific user.")
        visible: typeaheadView.count === 0
    }

    Component.onCompleted: {
        searchInput.forceActiveFocus()
    }
}
