import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Page {
    required property detailedprofile author

    signal closed

    id: page

    Image {
        id: bannerImg
        anchors.top: parent.top
        width: parent.width
        source: author.banner
        fillMode: Image.PreserveAspectFit

        SvgButton {
            anchors.top: parent.top
            anchors.left: parent.left
            iconColor: "white"
            Material.background: "transparent"
            svg: svgOutline.arrowBack
            onClicked: page.closed()
        }
    }

    Rectangle {
        id: avatar
        x: parent.width - width - 10
        y: bannerImg.y + bannerImg.height - height / 2
        width: 104
        height: width
        radius: width / 2
        color: "white"

        Avatar {
            anchors.centerIn: parent
            width: parent.width - 4
            height: parent.height - 4
            avatarUrl: author.avatarUrl
        }
    }

    Column {
        anchors.top: avatar.bottom
        width: parent.width
        padding: 10
        spacing: 10

        Text {
            id: nameText
            width: parent.width - 2 * parent.padding
            elide: Text.ElideRight
            font.pointSize: guiSettings.scaledFont(20/8)
            text: author.displayName
        }

        Text {
            id: handleText
            width: parent.width - 2 * parent.padding
            elide: Text.ElideRight
            color: guiSettings.handleColor
            text: `@${author.handle}`
        }

        Row {
            id: statsRow
            width: parent.width - 2 * parent.padding
            spacing: 20

            Text {
                color: guiSettings.linkColor
                text: qsTr(`<b>${author.followersCount}</b> followers`)
            }
            Text {
                color: guiSettings.linkColor
                text: qsTr(`<b>${author.followsCount}</b> following`)
            }
            Text {
                color: guiSettings.linkColor
                text: qsTr(`<b>${author.postsCount}</b> posts`)
            }
        }

        Text {
            id: descriptionText
            width: parent.width - 2 * parent.padding
            wrapMode: Text.Wrap
            text: author.description
        }
    }

    GuiSettings {
        id: guiSettings
    }
}
