import QtQuick
import QtQuick.Controls

ScrollView {
    required property list<string> contentLabels

    width: Math.min(parent.width, labelRow.width)
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
                backgroudColor: "lightgrey"
                font.pointSize: guiSettings.scaledFont(5/8)
                font.italic: true
                text: modelData
            }
        }
    }
}
