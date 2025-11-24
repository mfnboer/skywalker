import QtQuick
import skywalker

SkyPage {
    required property string dumpData

    signal closed()

    id: page
    width: parent.width
    height: parent.height

    header: SimpleHeader {
        text: title
        onBack: closed()

        SkyButton {
            y: (parent.height - height) / 2
            anchors.right: parent.right
            anchors.rightMargin: 10
            text: "Copy"
            onClicked: root.getSkywalker().copyToClipboard(dumpData)
        }
    }

    Flickable {
        anchors.fill: parent
        width: parent.width
        clip: true
        contentWidth: parent.width
        contentHeight: dumpText.height
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds

        AccessibleText {
            id: dumpText
            padding: 10
            width: parent.width
            wrapMode: Text.Wrap
            text: dumpData
        }
    }
}
