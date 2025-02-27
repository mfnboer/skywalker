import QtQuick
import QtQuick.Controls
import skywalker

TabButton {
    required property favoritefeedview favorite
    property string backgroundColor: guiSettings.backgroundColor

    id: button
    implicitWidth: tabRow.implicitWidth + leftPadding + rightPadding
    display: AbstractButton.TextOnly
    Accessible.name: qsTr(`Press to show ${text}`)

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

            onClicked: button.clicked()
        }

        Text {
            id: tabText
            anchors.verticalCenter: parent.verticalCenter
            font: button.font
            color: button.checked ? guiSettings.accentColor : guiSettings.textColor
            text: button.text
        }
    }

    background: Rectangle {
        anchors.fill: parent
        color: backgroundColor
    }
}
