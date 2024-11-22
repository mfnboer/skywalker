import QtQuick
import skywalker

SkyPage {
    property var skywalker: root.getSkywalker()

    signal closed
    signal selected(var focusHashtag)

    id: page
    clip: true

    header: SimpleHeader {
        text: qsTr("Add focus hashtag view")
        backIsCancel: true
        onBack: closed()
    }

    SkyListView {
        anchors.fill: parent
        boundsBehavior: Flickable.StopAtBounds
        model: skywalker.focusHashtags.entries

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
                text: getEntryText(modelData)
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
    }

    // TODO: move to GuiSettings?
    function getEntryText(entry) {
        let text = ""

        for (let i = 0; i < entry.hashtags.length; ++i) {
            const tag = entry.hashtags[i]

            if (i > 0)
                text += ' '

            text += `<a href="${tag}" style="color: ${guiSettings.linkColor}; text-decoration: none">#${tag}</a>`
        }

        console.debug("TEXT:", text)
        return text
    }
}
