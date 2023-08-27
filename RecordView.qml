import QtQuick
import QtQuick.Layouts
import skywalker

Item {
    property recordview record

    width: parent.width
    height: recordColumn.height + 10

    Column {
        id: recordColumn
        width: parent.width - 10
        anchors.centerIn: parent

        RowLayout {
            width: parent.width

            Avatar {
                id: avatar
                width: 15
                Layout.alignment: Qt.AlignTop
                avatarUrl: record.author.avatarUrl
            }

            PostHeader {
                Layout.fillWidth: true
                authorName: record.author.name
                postCreatedSecondsAgo: record.createdSecondsAgo
            }
        }

        PostBody {
            width: parent.width
            postText: record.postText
            postImages: record.images
            postExternal: record.external
            maxTextLines: 6
        }
    }
    Rectangle {
        anchors.fill: parent
        border.width: 1
        border.color: "lightgrey"
        color: "transparent"
        radius: 10
    }
}
