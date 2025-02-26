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
            unknownSvg: getDefaultAvatar()
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

    function getDefaultAvatar() {
        if (favorite.isNull())
            return SvgFilled.home

        switch (favorite.type) {
        case QEnums.FAVORITE_FEED:
            return guiSettings.feedDefaultAvatar(favorite.generatorView)
        case QEnums.FAVORITE_LIST:
            return SvgFilled.list
        case QEnums.FAVORITE_SEARCH:
            return favorite.searchFeed.isHashtag() ? SvgOutline.hashtag : SvgOutline.search
        }

        return SvgOutline.feed
    }
}
