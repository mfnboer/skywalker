import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import skywalker

AccessibleTabButton {
    property int counter: 0

    id: button
    implicitWidth: tabRow.implicitWidth + leftPadding + rightPadding

    contentItem: Row {
        id: tabRow
        height: parent.height
        spacing: 0

        AccessibleText {
            id: tabText
            anchors.verticalCenter: parent.verticalCenter
            elide: Text.ElideRight
            font: button.font
            color: button.checked ? guiSettings.accentColor : guiSettings.textColor
            text: button.text
        }

        BadgeCounter {
            id: badge
            counter: button.counter
        }
    }
}
