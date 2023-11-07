import QtQuick
import QtQuick.Controls

ScrollView {
    required property list<string> contentLabels

    width: Math.min(parent.width, labelRow.width)
    height: labelRow.height
    anchors.right: parent.right
    contentWidth: labelRow.width

    Row {
        id: labelRow
        topPadding: 5
        spacing: 5
        visible: contentLabels.length > 0

        Repeater {
            model: contentLabels

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

    Component.onCompleted: {
        console.debug("LABELS:", contentLabels)
    }
}
