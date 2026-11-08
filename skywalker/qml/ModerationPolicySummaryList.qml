import QtQuick
import skywalker

Repeater {
    required property list<ozonepolicysummary> summary

    id: list
    model: summary

    Column {
        required property ozonepolicysummary modelData

        x: list.x
        width: list.width
        spacing: 0

        AccessibleText {
            width: parent.width
            wrapMode: Text.Wrap
            topPadding: 10
            text: modelData.decription
        }
        AccessibleText {
            width: parent.width
            color: guiSettings.linkColor
            text: modelData.link
            visible: modelData.link

            MouseArea {
                anchors.fill: parent
                onClicked: root.openLink(modelData.link)
            }
        }
    }
}
