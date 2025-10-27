import QtQuick
import skywalker

SkyPage {
    property var skywalker: root.getSkywalker()
    readonly property string sideBarTitle: qsTr("Add focus hashtag view")
    readonly property SvgImage sideBarSvg: SvgOutline.hashtag

    signal closed
    signal selected(var focusHashtag)

    id: page
    padding: 10
    clip: true

    header: SimpleHeader {
        text: sideBarTitle
        backIsCancel: true
        visible: !root.showSideBar
        onBack: closed()
    }

    SkyListView {
        id: focusHashtagView
        anchors.fill: parent
        anchors.topMargin: !root.showSideBar ? 0 : guiSettings.headerMargin
        boundsBehavior: Flickable.StopAtBounds
        model: skywalker.focusHashtags.entries

        header: AccessibleText {
            width: parent.width
            padding: 10
            font.italic: true
            wrapMode: Text.Wrap
            text: qsTr("A focus hashtag view shows posts from your timeline that have one of the has tags from the focus.")
        }

        delegate: Rectangle {
            required property var modelData // FocusHashtagEntry

            width: parent.width
            height: focusEntry.height
            color: "transparent"

            AccessibleText {
                id: focusEntry
                width: parent.width
                padding: 10
                elide: Text.ElideRight
                wrapMode: Text.Wrap
                textFormat: Text.RichText
                font.pointSize: guiSettings.scaledFont(9/8)
                color: guiSettings.textColor
                text: guiSettings.getFocusHashtagEntryText(modelData)
            }

            Rectangle {
                anchors.fill: parent
                z: parent.z - 1
                color: modelData.highlightColor
                opacity: guiSettings.focusHighlightOpacity
            }

            MouseArea {
                anchors.fill: parent
                onClicked: page.selected(modelData)
            }
        }

        EmptyListIndication {
            id: emptyListIndication
            y: parent.headerItem ? parent.headerItem.height : 0
            svg: SvgOutline.hashtag
            text: qsTr("You have no focus hashtags. Create them in Settings > Moderation")
            list: focusHashtagView
        }
    }
}
