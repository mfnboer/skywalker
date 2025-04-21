import QtQuick
import skywalker

SkyPage {
    property var skywalker: root.getSkywalker()
    readonly property string sideBarTitle: qsTr("Add user view")
    readonly property SvgImage sideBarSvg: SvgOutline.user


    signal closed
    signal selected(basicprofile profile)

    id: page
    clip: true

    header: SimpleHeader {
        text: sideBarTitle
        backIsCancel: true
        visible: root.isPortrait
        onBack: closed()
    }

    SkyTextInput {
        id: searchInput
        width: parent.width
        svgIcon: SvgOutline.user
        placeholderText: qsTr("Search user")
        inputMethodHints: Qt.ImhNoAutoUppercase | Qt.ImhNoPredictiveText

        onDisplayTextChanged: {
            authorTypeaheadSearchTimer.start()
        }

        onEditingFinished: {
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

        onAuthorClicked: (profile) => selected(profile)
    }

    AccessibleText {
        x: 10
        anchors.top: searchInput.bottom
        topPadding: 10
        width: parent.width - 20
        font.italic: true
        wrapMode: Text.Wrap
        text: qsTr("A user view shows posts from your timeline posted by a specific user.")
        visible: typeaheadView.count === 0
    }

    Timer {
        id: authorTypeaheadSearchTimer
        interval: 500
        onTriggered: {
            const text = searchInput.displayText

            if (text.length > 0)
                searchUtils.searchAuthorsTypeahead(text, 100)
            else
                searchUtils.authorTypeaheadList = []
        }
    }

    SearchUtils {
        id: searchUtils
        skywalker: page.skywalker // qmllint disable missing-type
    }

    Component.onCompleted: {
        searchInput.forceActiveFocus()
    }
}
