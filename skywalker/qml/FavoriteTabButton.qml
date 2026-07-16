import QtQuick
import QtQuick.Controls
import skywalker

AccessibleTabButton {
    required property favoritefeedview favorite
    property int counter: 0

    id: button
    implicitWidth: tabRow.implicitWidth + leftPadding + rightPadding
    text: favorite.isNull() ? qsTr("Following", "timeline title") : favorite.name

    contentItem: Row {
        id: tabRow
        height: parent.height

        FeedAvatar {
            id: avatar
            anchors.verticalCenter: parent.verticalCenter
            width: visible ? parent.height : 0
            avatarUrl: favorite.avatarThumb
            unknownSvg: guiSettings.favoriteDefaultAvatar(favorite)
            contentMode: favorite.contentMode

            onClicked: button.click()
        }

        Column {
            width: tabText.width
            anchors.verticalCenter: parent.verticalCenter

            Text {
                id: tabText
                leftPadding: 5
                font: button.font
                color: button.checked ? guiSettings.accentColor : guiSettings.textColor
                text: button.text
            }

            Text {
                width: parent.width
                leftPadding: 5
                elide: Text.ElideRight
                font.pointSize: guiSettings.scaledFont(5/8)
                text: favorite.subTitle
                visible: favorite.subTitle
            }
        }

        BadgeCounter {
            counter: button.counter
        }
    }
}
