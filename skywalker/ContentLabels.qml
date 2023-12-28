import QtQuick
import QtQuick.Controls

ScrollView {
    required property list<string> contentLabels
    readonly property list<string> nonSystemLabels: filterSystemLabels()

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
                required property string modelData
                backgroundColor: guiSettings.contentLabelColor
                font.pointSize: guiSettings.scaledFont(5/8)
                font.italic: true
                color: guiSettings.textColor
                text: modelData
            }
        }
    }

    GuiSettings {
        id: guiSettings
    }

    function filterSystemLabels() {
        let labels = []

        for (let i = 0; i < contentLabels.length; ++i) {
            if (!contentLabels[i].startsWith("!"))
                labels.push(contentLabels[i])
        }

        return labels
    }
}
