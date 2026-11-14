import QtQuick
import QtQuick.Layouts
import skywalker

GridLayout {
    required property profilelinkview link
    property string backgroundColor: guiSettings.backgroundColor

    id: grid
    columns: 2
    rowSpacing: 0
    columnSpacing: 10

    RoundCornerMask {
        Layout.rowSpan: 2
        Layout.preferredWidth: 34
        Layout.preferredHeight: Layout.preferredWidth
        maskColor: grid.backgroundColor

        ImageAutoRetry {
            anchors.fill: parent
            fillMode: Image.PreserveAspectFit
            source: link.icon
            visible: link.icon
        }

        AccessibleText {
            anchors.fill: parent
            font.pointSize: height > 0 ? guiSettings.scaledFont(height / guiSettings.appFontHeight) : 1
            text: "🌐"
            visible: !link.icon
        }
    }

    AccessibleText {
        Layout.fillWidth: true
        font.bold: true
        text: link.title
    }

    AccessibleText {
        Layout.fillWidth: true
        text: guiSettings.toLinkDisplayText(link.url)
    }
}
