import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import skywalker

AccessibleTabButton {
    property basicprofile profile
    property int counter: 0
    property bool showWarning: false

    id: button
    implicitWidth: tabRow.implicitWidth + leftPadding + rightPadding

    contentItem: Row {
        id: tabRow
        height: parent.height
        spacing: 0

        Avatar {
            id: avatar
            anchors.verticalCenter: parent.verticalCenter
            width: parent.height
            author: profile
            showFollowingStatus: false
            onClicked: button.click()
        }

        AccessibleText {
            id: tabText
            width: Math.min(implicitWidth, 150)
            anchors.verticalCenter: parent.verticalCenter
            leftPadding: 5
            elide: Text.ElideRight
            font: button.font
            color: button.checked ? guiSettings.accentColor : guiSettings.textColor
            text: (showWarning ? "⚠ " : "") + '@' + profile.handle
        }

        BadgeCounter {
            id: badge
            counter: button.counter
        }
    }
}
