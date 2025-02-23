import QtQuick
import QtQuick.Controls
import skywalker

TabButton {
    required property favoritefeedview favorite

    id: button
    implicitWidth: tabRow.implicitWidth + leftPadding + rightPadding
    display: AbstractButton.TextOnly
    Accessible.name: qsTr(`Press to show ${text}`)

    text: favorite.name

    contentItem: Row {
        id: tabRow
        height: parent.height

        FeedAvatar {
            id: avatar
            anchors.verticalCenter: parent.verticalCenter
            width: visible ? parent.height : 0
            avatarUrl: favorite.avatarThumb
            unknownSvg: getDefaultAvatar()
            contentMode: favorite.contentMode
        }

        Rectangle {
            id: whitespace
            width: 5
            height: parent.height
            color: "transparent"
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
        color: parent.backgroundColor
    }

    function getDefaultAvatar() {
        switch (favorite.type) {
        case QEnums.FAVORITE_FEED:
            return guiSettings.feedDefaultAvatar(favorite.generatorView)
        case QEnums.FAVORITE_LIST:
            return SvgFilled.list
        case QEnums.FAVORITE_SEARCH:
            return modelData.searchFeed.isHashtag() ? SvgOutline.hashtag : SvgOutline.search
        }

        return SvgOutline.feed
    }
}
