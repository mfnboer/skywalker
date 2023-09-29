import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import skywalker

Page {
    required property var skywalker
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
            Material.background: "black"
            opacity: 0.5
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
            onClicked: root.viewFullImage([author.imageView], 0)
        }
    }

    Column {
        anchors.top: avatar.bottom
        width: parent.width
        leftPadding: 10
        rightPadding: 10

        Text {
            id: nameText
            width: parent.width - 2 * parent.padding
            elide: Text.ElideRight
            font.pointSize: guiSettings.scaledFont(16/8)
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
            spacing: 15
            topPadding: 10

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
            topPadding: 10
            wrapMode: Text.Wrap
            textFormat: Text.RichText
            text: postUtils.linkiFy(author.description)

            onLinkActivated: (link) => {
                if (link.startsWith("@"))
                    console.debug("TODO MENTION:", link)
                else
                    Qt.openUrlExternally(link)
            }
        }
    }

    PostUtils {
        id: postUtils
        skywalker: page.skywalker
    }

    GuiSettings {
        id: guiSettings
    }
}
