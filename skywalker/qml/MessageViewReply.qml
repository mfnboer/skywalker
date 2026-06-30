import QtQuick
import QtQuick.Controls
import skywalker

Rectangle {
    required property int maxWidth
    property int minWidth: 20
    required property convoview convo
    required property messageview replyTo
    property basicprofile author: convo.getMember(replyTo.senderDid).basicProfile
    property string backgroundColor: guiSettings.backgroundColor
    property string borderColor: guiSettings.isLightMode ? Qt.darker(backgroundColor, 1.1) : Qt.lighter(backgroundColor, 1.6)
    property Skywalker skywalker: root.getSkywalker()

    signal clicked

    id: view
    width: Math.max(viewColumn.width, minWidth)
    height: viewColumn.height + 20
    border.width: 1
    border.color: view.borderColor
    color: view.backgroundColor
    radius: guiSettings.radius

    MouseArea {
        anchors.fill: parent
        onClicked: view.clicked()
    }

    Column {
        id: viewColumn
        y: 10
        width: Math.max(Math.min(authorName.advanceWidth + 20, maxWidth), messageText.width)

        SkyCleanedTextLine {
            id: authorName
            width: view.maxWidth
            leftPadding: 10
            rightPadding: 10
            elide: Text.ElideRight
            color: Material.color(Material.Grey)
            font.bold: true
            font.pointSize: guiSettings.scaledFont(7/8)
            plainText: view.author.name
            visible: replyTo.senderDid
        }

        MessageText {
            id: messageText
            maxWidth: view.maxWidth
            convo: view.convo
            message: view.replyTo
            author: view.author
            maximumLineCount: 3
            ellipsisBackgroundColor: view.backgroundColor
            font.pointSize: guiSettings.scaledFont(7/8)
            showEmbedTextIfMessageIsEmpty: true
        }
    }

    Loader {
        id: profileLoader
        active: author.isNull() && replyTo.senderDid

        sourceComponent: ProfileUtils {
            skywalker: view.skywalker

            onBasicProfileOk: (profile) => view.author = profile
        }

        onStatusChanged: {
            if (status == Loader.Ready)
                item.getBasicProfile(replyTo.senderDid)
        }
    }
}
