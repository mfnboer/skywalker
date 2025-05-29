import QtQuick
import skywalker

SkyPage {
    property var skywalker: root.getSkywalker()
    readonly property string sideBarTitle: qsTr("Add hashtag view")
    readonly property SvgImage sideBarSvg: SvgOutline.hashtag

    signal closed
    signal selected(string hashtag)

    id: page
    clip: true

    header: SimpleHeader {
        text: sideBarTitle
        backIsCancel: true
        visible: !root.showSideBar
        onBack: closed()

        SvgPlainButton {
            id: okButton
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            iconColor: enabled ? guiSettings.buttonColor : guiSettings.disabledColor
            svg: SvgOutline.check
            accessibleName: qsTr("ok")
            onClicked: page.selected(page.getHashtag())
            enabled: searchInput.text.length > (searchInput.text.startsWith("#") ? 1 : 0)
        }
    }

    SkyTextInput {
        id: searchInput
        y: !root.showSideBar ? 0 : guiSettings.headerMargin
        width: parent.width
        svgIcon: SvgOutline.hashtag
        placeholderText: qsTr("Search hashtag")
        validator: RegularExpressionValidator { regularExpression: /[^ ]+/ }

        onDisplayTextChanged: {
            hashtagTypeaheadSearchTimer.start()
        }

        onEditingFinished: {
            hashtagTypeaheadSearchTimer.stop()
        }
    }

    HashtagListView {
        id: hashtagTypeaheadView
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: searchInput.bottom
        anchors.bottom: parent.bottom
        model: searchUtils.hashtagTypeaheadList

        onHashtagClicked: (tag) => {
            searchInput.text = `#${tag}`
        }
    }

    AccessibleText {
        x: 10
        anchors.top: searchInput.bottom
        topPadding: 10
        width: parent.width - 20
        font.italic: true
        wrapMode: Text.Wrap
        text: qsTr("A hashtag view shows posts from your timeline that contain this hashtag.")
        visible: hashtagTypeaheadView.count === 0
    }

    Timer {
        id: hashtagTypeaheadSearchTimer
        interval: 500
        onTriggered: {
            let text = searchInput.displayText

            if (text.startsWith('#'))
                text = text.slice(1) // strip #-symbol

            if (text.length > 0)
                searchUtils.searchHashtagsTypeahead(text, 100)
            else
                searchUtils.hashtagTypeaheadList= []
        }
    }

    SearchUtils {
        id: searchUtils
        skywalker: page.skywalker // qmllint disable missing-type
    }

    function getHashtag() {
        let text = searchInput.text.trim()

        if (text.startsWith('#'))
            text = text.slice(1) // strip #-symbol

        return text
    }


    Component.onCompleted: {
        searchInput.forceActiveFocus()
    }
}
