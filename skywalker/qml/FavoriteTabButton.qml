import QtQuick
import QtQuick.Controls
import skywalker

AccessibleTabButton {
    required property favoritefeedview favorite

    id: button
    implicitWidth: tabRow.implicitWidth + leftPadding + rightPadding
    text: favorite.isNull() ? qsTr("Following", "timeline title") : favorite.name

    contentItem: Row {
        id: tabRow
        height: parent.height
        spacing: 5

        FeedAvatar {
            id: avatar
            anchors.verticalCenter: parent.verticalCenter
            width: visible ? parent.height : 0
            avatarUrl: favorite.avatarThumb
            unknownSvg: guiSettings.favoriteDefaultAvatar(favorite)
            contentMode: favorite.contentMode

            onClicked: button.click()
        }

        Text {
            id: tabText
            anchors.verticalCenter: parent.verticalCenter
            font: button.font
            color: button.checked ? guiSettings.accentColor : guiSettings.textColor
            text: button.text
        }
    }
}
