import QtQuick
import QtQuick.Controls
import skywalker

ScrollView {
    required property list<contentlabel> contentLabels
    readonly property list<contentlabel> nonSystemLabels: filterSystemLabels()

    width: Math.min(parent.width, labelRow.width)
    height: labelRow.height
    anchors.right: parent.right
    contentWidth: labelRow.width

    Row {
        id: labelRow
        topPadding: 5
        spacing: 5
        visible: nonSystemLabels.length > 0

        Repeater {
            model: nonSystemLabels

            SkyLabel {
                required property contentlabel modelData
                backgroundColor: guiSettings.contentLabelColor
                font.pointSize: guiSettings.scaledFont(5/8)
                font.italic: true
                color: guiSettings.textColor
                text: modelData.text
            }
        }
    }

    GuiSettings {
        id: guiSettings
    }

    function filterSystemLabels() {
        let labels = []

        for (let i = 0; i < contentLabels.length; ++i) {
            if (!contentLabels[i].isSystemLabel())
                labels.push(contentLabels[i])
        }

        return labels
    }
}
