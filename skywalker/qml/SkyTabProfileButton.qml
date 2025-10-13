import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import skywalker

AccessibleTabButton {
    property basicprofile profile
    property int counter: 0

    signal closed

    id: button
    implicitWidth: tabRow.implicitWidth + leftPadding + rightPadding

    contentItem: Row {
        id: tabRow
        height: parent.height
        spacing: 5

        Avatar {
            id: avatar
            anchors.verticalCenter: parent.verticalCenter
            width: parent.height
            author: profile
            onClicked: button.click()
        }

        Text {
            id: tabText
            width: Math.min(implicitWidth, 150)
            anchors.verticalCenter: parent.verticalCenter
            elide: Text.ElideRight
            font: button.font
            color: button.checked ? guiSettings.accentColor : guiSettings.textColor
            text: '@' + profile.handle
        }

        BadgeCounter {
            id: badge
            color: guiSettings.backgroundColor
            counterColor: tabText.color
            counter: button.counter
        }
    }
}
